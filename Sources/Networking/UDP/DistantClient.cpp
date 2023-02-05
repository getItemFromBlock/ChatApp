#include "Networking/UDP/DistantClient.hpp"

#include "Networking/Messages.hpp"
#include "Networking/Utils.hpp"
#include "Networking/UDP/Client.hpp"

namespace Networking::UDP
{
	std::chrono::milliseconds DistantClient::sTimeout = UDP_TIMEOUT;

	DistantClient::DistantClient(Client& client, const Address& address, u64 clientID) : 
		mClient(client), mAddress(address), mClientId(clientID), mConnectionStartTime(Utils::Now()), mLastKeepAlive(Utils::Now())
	{
	}

	void DistantClient::onConnectionSent()
	{
		if (mState == State::None)
		{
			mState = State::ConnectionSent;
			maintainConnection();
		}
		else if (mState == State::ConnectionReceived)
		{
			// We sent a connection and had received one : we do connect !
			onConnected();
		}
	}

	void DistantClient::send(std::vector<uint8_t>&& data, u32 canalIndex)
	{
		onConnectionSent();
		mChannelsHandler.queue(std::move(data), canalIndex);
	}

	void DistantClient::fillDatagramHeader(Datagram& dgram, Datagram::Type type)
	{
		dgram.header.ack = htons(mReceivedAcks.lastAck());
		dgram.header.previousAcks = mReceivedAcks.previousAcksMask();
		dgram.header.id = htons(mNextDatagramIdToSend);
		dgram.header.type = type;
		++mNextDatagramIdToSend;
	}

	void DistantClient::send(const Datagram& dgram)
	{
		int ret = mAddress.sendTo(mClient.mSocket, reinterpret_cast<const char*>(&dgram), dgram.size());
		if (ret < 0)
		{
			// Error
		}
	}

	void DistantClient::processSend(const u8 maxDatagrams)
	{
		const auto now = Utils::Now();
		// We do send data during connection process in order to keep it available before we accept it
		if (isConnecting() || isConnected())
		{
#if NETWORK_INTERRUPTION
			if (mClient.isNetworkInterrupted())
			{
				// Since the network is interrupted, send a keep alive to let the client know that
				Datagram datagram;
				fillKeepAlive(datagram);
				send(datagram);
			}
#endif
			for (size_t loop = 0; maxDatagrams == 0 || loop < maxDatagrams; ++loop)
			{
				Datagram datagram;
				datagram.datasize = mChannelsHandler.serialize(datagram.data.data(), Datagram::DataMaxSize, mNextDatagramIdToSend
#if NETWORK_INTERRUPTION
					, mClient.isNetworkInterrupted()
#endif
				);
				if (datagram.datasize > 0)
				{
					fillDatagramHeader(datagram, Datagram::Type::ConnectedData);
					send(datagram);
				}
				else
				{
					const bool sendKeepAlive = (loop == 0)
#if NETWORK_INTERRUPTION
						&& !mClient.isNetworkInterrupted()
#endif
						;
					if (sendKeepAlive)
					{
						// Nothing to send this time, so send a keep alive to maintain connection
						fillKeepAlive(datagram);
						send(datagram);
					}
					// Everything has been sent. Exit the sending loop.
					break;
				}
			}
		}
		if (isDisconnecting())
		{
			if (now > mLastKeepAlive + 2 * GetTimeout())
			{
				// After 2 timeouts we mark it disconnected
				// This leaves enough time to each end to notice and disconnects its distant client
#if NETWORK_INTERRUPTION
	// Resume it right before disconnection for the app to receive the corresponding messages
				if (isInterrupted())
					onConnectionResumed();
#endif
				// Do notify the disconnection, if needed
				// We notify it as latest as possible so when user received the Disconnection message, he can send a new connection request right away
				switch (mDisconnectionReason)
				{
				case DisconnectionReason::Disconnected:
				case DisconnectionReason::DisconnectedFromOtherEnd:
					onMessageReady(std::make_unique<Messages::Disconnection>(mAddress, mClientId, Messages::Disconnection::Reason::Disconnected));
					break;
				case DisconnectionReason::Lost:
					onMessageReady(std::make_unique<Messages::Disconnection>(mAddress, mClientId, Messages::Disconnection::Reason::Lost));
					break;
				case DisconnectionReason::Refused:
					onMessageReady(std::make_unique<Messages::Connection>(mAddress, mClientId, Messages::Connection::Result::Refused));
					break;
				case DisconnectionReason::ConnectionTimedOut:
					onMessageReady(std::make_unique<Messages::Connection>(mAddress, mClientId, Messages::Connection::Result::TimedOut));
					break;
				}
				mState = State::Disconnected;
			}
			else if (mDisconnectionReason != DisconnectionReason::None && mDisconnectionReason != DisconnectionReason::Lost)
			{
				// Send disconnection datagrams while disconnecting to inform the other end that's a normal termination
				Datagram datagram;
				fillDatagramHeader(datagram, Datagram::Type::Disconnection);
				send(datagram);
			}
		}
		else if (isConnected() && now > mLastKeepAlive + GetTimeout())
		{
			onConnectionInterrupted();
		}
		else if (isConnecting() && now > mConnectionStartTime + GetTimeout())
		{
			// Connection hasn't been accepted within timeframe : drop it, we don't keep an interrupted connection before it's been accepted
			onConnectionTimedOut();
		}
	}

	void DistantClient::fillKeepAlive(Datagram& dgram)
	{
		fillDatagramHeader(dgram, Datagram::Type::KeepAlive);
		// Do notify the other end if we're supposed to be connected or requesting the connection
		u8 data = 0;
		data |= (mState == State::ConnectionSent || isConnected());
#if NETWORK_INTERRUPTION
		// If the connection is interrupted with that client, check whether another client is also interrupting it or not
		// If this client is the only one causing the interruption, don't send him an interruption flag so he doesn't interrupt himself when resuming on his side
		const bool isNetworkInterrupted = mClient.isNetworkInterrupted();
		const bool isNetworkInterruptedByMe = mClient.isInterruptionCulprit(this);
		data |= (isNetworkInterrupted && !isNetworkInterruptedByMe) << 1;
#endif
		memcpy(dgram.data.data(), &data, 1);
		dgram.datasize = 1;
	}

	void DistantClient::handleKeepAlive(const u8* data, const u16 datasize)
	{
		std::vector<u8> content = std::vector<u8>(datasize);
		memcpy(content.data(), data, datasize);
		u8 isConnectedKeepAlive = content[0];
		if (isConnectedKeepAlive & 0x01)
		{
			if (mState == State::None || isConnecting())
			{
				onConnectionReceived();
			}
		}

#if NETWORK_INTERRUPTION
		bool isNetworkInterruptedOnTheOtherEnd = false;
		// Retrieve whether the other side has its connection interrupted and we should locally interrupt it too
		isNetworkInterruptedOnTheOtherEnd = content.size() > 0 && isConnectedKeepAlive & 0x02;
		// Always consider the connection OK when a keep alive is received, but do keep in mind the network may be interrupted because it's interrupted on the other end.
		maintainConnection(isNetworkInterruptedOnTheOtherEnd);
#else
		maintainConnection();
#endif
	}

	void DistantClient::onDatagramReceived(Datagram&& datagram)
	{
		const auto datagramid = ntohs(datagram.header.id);
		//!< Update the received acks tracking
		mReceivedAcks.update(datagramid, 0, true);
		//!< Update the send acks tracking
		mSentAcks.update(ntohs(datagram.header.ack), datagram.header.previousAcks, true);
		//!< Ignore duplicate
		if (!mReceivedAcks.isNewlyAcked(datagramid))
		{
			return;
		}

		//!< Handle loss on reception
		const auto lostDatagrams = mReceivedAcks.loss();
		for (const auto receivedLostDatagram : lostDatagrams)
		{
			onDatagramReceivedLost(receivedLostDatagram);
		}
		//!< Handle loss on send
		const auto datagramsSentLost = mSentAcks.loss();
		for (const auto sendLoss : datagramsSentLost)
		{
			onDatagramSentLost(sendLoss);
		}
		//!< Mark new send acked
		const auto datagramsSentAcked = mSentAcks.getNewAcks();
		for (const auto sendAcked : datagramsSentAcked)
		{
			onDatagramSentAcked(sendAcked);
		}
		switch (datagram.header.type)
		{
		case Datagram::Type::ConnectedData:
		{
			//!< Dispatch data
			onDataReceived(datagram.data.data(), datagram.datasize);
		} break;
		case Datagram::Type::KeepAlive:
		{
			handleKeepAlive(datagram.data.data(), datagram.datasize);
		} break;
		case Datagram::Type::Disconnection:
		{
			onDisconnectionFromOtherEnd();
		} break;
		}
	}

	void DistantClient::onDatagramSentAcked(Datagram::ID datagramId)
	{
		mChannelsHandler.onDatagramAcked(datagramId);
	}

	void DistantClient::onDatagramSentLost(Datagram::ID datagramId)
	{
		mChannelsHandler.onDatagramLost(datagramId);
	}

	void DistantClient::onDatagramReceivedLost(Datagram::ID)
	{
	}

	void DistantClient::onDataReceived(const u8* data, const u16 datasize)
	{
		// If we receive data, the other end is requesting a connection
		onConnectionReceived();
		mChannelsHandler.onDataReceived(data, datasize);
		auto receivedMessages = mChannelsHandler.process(isConnected());
		for (auto&& [channelId, msg] : receivedMessages)
		{
			onMessageReady(std::make_unique<Messages::UserData>(mAddress, mClientId, std::move(msg), channelId));
		}
		maintainConnection();
	}

	void DistantClient::onMessageReady(std::unique_ptr<Messages::Base>&& msg)
	{
		if (isConnecting())
		{
			mPendingMessages.push_back(std::move(msg));
		}
		else if (isConnected() || isDisconnecting())
		{
			mClient.onMessageReady(std::move(msg));
		}
	}

	void DistantClient::onDisconnectionFromOtherEnd()
	{
		if (isConnecting())
		{
			onConnectionRefused();
		}
		else if (isConnected())
		{
			mState = State::Disconnecting;
			mDisconnectionReason = DisconnectionReason::DisconnectedFromOtherEnd;
		}
	}

	void DistantClient::onConnectionLost()
	{
		// Start disconnecting and save the reason to notify later
		mState = State::Disconnecting;
		mDisconnectionReason = DisconnectionReason::Lost;
	}

	void DistantClient::onConnectionRefused()
	{
		if (mState == State::ConnectionSent)
		{
			mDisconnectionReason = DisconnectionReason::Refused;
		}
		mState = State::Disconnecting;
	}

	void DistantClient::onConnectionTimedOut()
	{
		if (mState == State::ConnectionSent)
		{
			mDisconnectionReason = DisconnectionReason::ConnectionTimedOut;
		}
		mState = State::Disconnecting;
	}

	void DistantClient::onConnected()
	{
		mState = State::Connected;
		maintainConnection();
		onMessageReady(std::make_unique<Messages::Connection>(mAddress, mClientId, Messages::Connection::Result::Success));
		// Transfert des messages en attente !
		for (auto&& pendingMessage : mPendingMessages)
		{
			onMessageReady(std::move(pendingMessage));
		}
	}

	void DistantClient::onConnectionReceived()
	{
		if (mState == State::None)
		{
			mState = State::ConnectionReceived;
			maintainConnection();
			// Push incoming connection request to client
			mClient.onMessageReady(std::make_unique<Messages::IncomingConnection>(mAddress, mClientId));
		}
		else if (mState == State::ConnectionSent)
		{
			// We received a connection and had sent one : we do connect !
			onConnected();
		}
	}

	void DistantClient::onConnectionInterrupted()
	{
#if NETWORK_INTERRUPTION
		if (mClient.isNetworkInterruptionAllowed())
		{
			if (!mInterrupted)
			{
				mInterrupted = true;
				mClient.onClientInterrupted(this);
				onMessageReady(std::make_unique<Messages::ConnectionInterrupted>(mAddress, mClientId, true));
			}
		}
		else
#endif
		{
			if (isConnected())
			{
				onConnectionLost();
			}
		}
	}

#if NETWORK_INTERRUPTION
	void DistantClient::onConnectionInterruptedForwarded()
	{
		if (mClient.isNetworkInterruptionAllowed())
		{
			if (!mDistantInterrupted)
			{
				// If we were interrupted, we must have received something to know the distant interruption
				if (mInterrupted)
				{
					mInterrupted = false;
					onMessageReady(std::make_unique<Messages::ConnectionResumed>(mAddress, mClientId, false));
				}
				mDistantInterrupted = true;
				mClient.onClientInterrupted(this);
				onMessageReady(std::make_unique<Messages::ConnectionInterrupted>(mAddress, mClientId, false));
			}
		}
	}

	void DistantClient::onConnectionResumed()
	{
		if (mInterrupted || mDistantInterrupted)
		{
			mInterrupted = false;
			mDistantInterrupted = false;
			mClient.onClientResumed(this);
			onMessageReady(std::make_unique<Messages::ConnectionResumed>(mAddress, mClientId, !mClient.isNetworkInterrupted()));
	}
			}
#endif

#if NETWORK_INTERRUPTION
	void DistantClient::maintainConnection(bool distantNetworkInterrupted)
#else
	void DistantClient::maintainConnection()
#endif
	{
		mLastKeepAlive = Utils::Now();
#if NETWORK_INTERRUPTION
		if (distantNetworkInterrupted)
			onConnectionInterruptedForwarded();
		else
			onConnectionResumed();
#endif
	}

	void DistantClient::connect()
	{
		onConnectionSent();
	}

	void DistantClient::disconnect()
	{
		// To disconnect, stop sending packets
		// We switch to a disconnecting state to prevent a reconnection from happening
		// when receiving packets from the other end right after disconnecting locally
		mDisconnectionReason = DisconnectionReason::Disconnected;
		mState = State::Disconnecting;
		mLastKeepAlive = Utils::Now();
	}
}