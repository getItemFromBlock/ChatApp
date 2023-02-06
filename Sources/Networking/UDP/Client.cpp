#include "Networking/UDP/Client.hpp"

#include "Networking/UDP/Datagram.hpp"
#include "Networking/Messages.hpp"
#include "Networking/UDP/DistantClient.hpp"
#include "Networking/Errors.hpp"

namespace Networking::UDP
{
	void Client::SetTimeout(std::chrono::milliseconds timeout)
	{
		DistantClient::SetTimeout(timeout);
	}

	std::chrono::milliseconds Client::GetTimeout()
	{
		return DistantClient::GetTimeout();
	}

	Client::Client()
	{
	}

	Client::~Client()
	{
		release();
	}

	bool Client::init(const u16 port)
	{
		assert(!mRegisteredChannels.empty()); // Initializing without any channel doesn't make sense..

		release();
		mSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (mSocket == INVALID_SOCKET)
			return false;

		const Address addr = Address::Any(Address::Type::IPv4, port);
		if (!addr.bind(mSocket))
			return false;

		if (!SetNonBlocking(mSocket))
			return false;

		mClientIdsGenerator = 0;
		return true;
	}
	void Client::release()
	{
		if (mSocket != INVALID_SOCKET)
			CloseSocket(mSocket);
		mSocket = INVALID_SOCKET;
		{
#if NETWORK_THREAD_SAFE
			OperationsLock lock(mOperationsLock);
#endif
			mPendingOperations.clear();
		}
		{
#if NETWORK_THREAD_SAFE
			MessagesLock lock(mMessagesLock);
#endif
			mMessages.clear();
		}
		mClients.clear();
#if NETWORK_INTERRUPTION
		mInterruptedClients.clear();
#endif
	}

	void Client::connect(const Address& addr)
	{
		assert(addr.isValid());
#if NETWORK_THREAD_SAFE
		OperationsLock lock(mOperationsLock);
#endif
		mPendingOperations.push_back(Operation::Connect(addr));
	}

	void Client::disconnect(const Address& addr)
	{
		assert(addr.isValid());
#if NETWORK_THREAD_SAFE
		OperationsLock lock(mOperationsLock);
#endif
		mPendingOperations.push_back(Operation::Disconnect(addr));
	}

	void Client::disconnectAll()
	{
#if NETWORK_THREAD_SAFE
		OperationsLock lock(mOperationsLock); $
#endif
			mPendingOperations.push_back(Operation::DisconnectAll());
	}

	void Client::sendTo(const Address& target, std::vector<u8>&& data, const u32 channelIndex)
	{
		assert(target.isValid());
#if NETWORK_THREAD_SAFE
		OperationsLock lock(mOperationsLock);$
#endif
		mPendingOperations.push_back(Operation::SendTo(target, std::move(data), channelIndex));
	}

	void Client::broadCast(std::vector<u8>&& data, u32 channelIndex)
	{
#if NETWORK_THREAD_SAFE
		OperationsLock lock(mOperationsLock); $
#endif
			mPendingOperations.push_back(Operation::BroadCast(std::move(data), channelIndex));
	}

	void Client::processSend()
	{
		// Process pending operations
		std::vector<Operation> operations;
		{
#if NETWORK_THREAD_SAFE
			OperationsLock lock(mOperationsLock);
#endif
			operations.swap(mPendingOperations);
		}
		for (Operation& op : operations)
		{
			switch (op.mType)
			{
			case Operation::Type::Connect:
			{
				if (auto client = getClient(op.mTarget, true))
					client->connect();
			} break;
			case Operation::Type::SendTo:
			{
				if (auto client = getClient(op.mTarget, true))
					client->send(std::move(op.mData), op.mChannel);
			} break;
			case Operation::Type::BroadCast:
			{
				for (auto& client : mClients)
				{
					client->send(std::move(op.mData), op.mChannel);
				}
			} break;
			case Operation::Type::Disconnect:
			{
				if (auto client = getClient(op.mTarget))
					client->disconnect();
			} break;
			case Operation::Type::DisconnectAll:
			{
				for (auto& client : mClients)
				{
					client->disconnect();
				}
			} break;
			}
		}

		// Do send data to clients
		for (auto& client : mClients)
			client->processSend();

		// Remove disconnected clients
		const auto clientsToRemove = std::remove_if(mClients.begin(), mClients.end(), [](const std::unique_ptr<DistantClient>& client) { return client->isDisconnected(); });
#if NETWORK_INTERRUPTION
		// Make sure no interrupted clients have been removed : interrupted clients should resume before disconnecting
		for (auto clientToRemove = clientsToRemove; clientToRemove != mClients.end(); ++clientToRemove)
		{
			const size_t erased = mInterruptedClients.erase(clientToRemove->get());
			assert(erased == 0);
		}
#endif
		mClients.erase(clientsToRemove, mClients.end());
	}

#if NETWORK_INTERRUPTION
	void Client::onClientInterrupted(const DistantClient* client)
	{
		mInterruptedClients.insert(client);
	}

	void Client::onClientResumed(const DistantClient* client)
	{
		mInterruptedClients.erase(client);
	}

	bool Client::isInterruptionCulprit(const DistantClient* client) const
	{
		return mInterruptedClients.size() == 1 && *(mInterruptedClients.begin()) == client;
	}

#endif
	void Client::receive()
	{
		for (;;)
		{
			Datagram datagram;
			Address from;
			int ret = from.recvFrom(mSocket, reinterpret_cast<u8*>(&datagram), Datagram::BufferMaxSize);
			if (ret > 0)
			{
				const u16 receivedSize = static_cast<u16>(ret);
				if (receivedSize >= Datagram::HeaderSize)
				{
					datagram.datasize = receivedSize - Datagram::HeaderSize;
#if NETWORK_SIMULATOR
					if (mSimulator.isEnabled())
					{
						// Push the datagram into the simulator
						mSimulator.push(datagram, from);
					}
					else
#endif
					{
						// Handle the datagram directly
						if (auto client = getClient(from, true))
							client->onDatagramReceived(std::move(datagram));
					}
				}
				else
				{
					//!< Something is wrong, unexpected datagram
					assert(0);
				}
			}
			else
			{
				if (ret < 0)
				{
					//!< Error handling
					const auto err = Sockets::GetErrorCasted();
					if (err != Sockets::Errors::WOULDBLOCK)
					{
						//!< Log that error
					}
				}
				break;
			}
		}
		// Poll pending datagrams from the simulator
#if NETWORK_SIMULATOR
		if (mSimulator.isEnabled())
		{
			std::vector<std::pair<Datagram, Address>> datagrams = mSimulator.poll();
			for (auto& [datagram, from] : datagrams)
			{
				if (auto client = getClient(from, true))
					client->onDatagramReceived(std::move(datagram));
			}
		}
#endif
	}

	std::vector<std::unique_ptr<Messages::Base>> Client::poll()
	{
#if NETWORK_THREAD_SAFE
		MessagesLock lock(mMessagesLock);
#endif
		return std::move(mMessages);
	}

	const Address& Client::GetClientAddress(u64 clientID)
	{
		for (auto& c : mClients)
		{
			if (c->id() != clientID) continue;
			return c->address();
		}
		return Address();
	}

	bool Client::IsClientDisconnected(const Address& clientAddr)
	{
		DistantClient* cl = getClient(clientAddr);
		return (!cl || cl->isDisconnected());
	}

	DistantClient* Client::getClient(const Address& clientAddr, bool create)
	{
		auto itClient = std::find_if(mClients.begin(), mClients.end(), [&](const std::unique_ptr<DistantClient>& client) { return client->address() == clientAddr; });
		if (itClient != mClients.end())
			return itClient->get();
		else if (create)
		{
			mClients.emplace_back(std::make_unique<DistantClient>(*this, clientAddr, mClientIdsGenerator++));
			setupChannels(*(mClients.back()));
			return mClients.back().get();
		}
		else
			return nullptr;
	}

	void Client::setupChannels(DistantClient& client)
	{
		for (auto& channel : mRegisteredChannels)
			channel.creator(client);
	}

	void Client::onMessageReady(std::unique_ptr<Messages::Base>&& msg)
	{
#if NETWORK_THREAD_SAFE
		MessagesLock lock(mMessagesLock);
#endif
		mMessages.push_back(std::move(msg));
	}

}