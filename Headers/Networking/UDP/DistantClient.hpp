#pragma once

#include <vector>
#include <chrono>

#include "Datagram.hpp"
#include "Networking/Sockets.hpp"
#include "AckHandler.hpp"
#include "ChannelsHandler.hpp"
#include "Networking/NetworkSettings.hpp"
#include "Networking/Messages.hpp"
#include "Networking/Address.hpp"

namespace Networking::UDP
{
	class Client;
	class DistantClient
	{
	public:
		DistantClient(Client& client, const Address& address, u64 clientID);
		DistantClient(const DistantClient&) = delete;
		DistantClient(DistantClient&&) = delete;
		DistantClient& operator=(const DistantClient&) = delete;
		DistantClient& operator=(DistantClient&&) = delete;
		~DistantClient() = default;

		static void SetTimeout(std::chrono::milliseconds timeout) { sTimeout = timeout; }
		static std::chrono::milliseconds GetTimeout() { return sTimeout; }

		void connect();
		void disconnect();
		void send(std::vector<uint8_t>&& data, u32 canalIndex);
		void processSend(u8 maxDatagrams = 0);
		void onDatagramReceived(Datagram&& datagram);
		bool isConnected() const { return mState == State::Connected; }
		bool isConnecting() const { return mState == State::ConnectionReceived || mState == State::ConnectionSent; }
		inline bool isDisconnecting() const { return mState == State::Disconnecting; }
		inline bool isDisconnected() const { return mState == State::Disconnected; }
#if NETWORK_INTERRUPTION
		inline bool isInterrupted() const { return mInterrupted; }
		inline bool hasInterruptedClients() const { return mDistantInterrupted; }
#endif

		const Address& address() const { return mAddress; }
		u64 id() const { return mClientId; }

		template<class T>
		void registerChannel(u8 channelId = 0)
		{
			mChannelsHandler.registerChannel<T>(channelId);
		}

	private:
		enum class State
		{
			None,
			Connected,
			ConnectionReceived,
			ConnectionSent,
			Disconnecting,
			Disconnected,
		};
		enum class DisconnectionReason
		{
			None,
			Refused,
			ConnectionTimedOut,
			Disconnected,
			DisconnectedFromOtherEnd,
			Lost,
		};
		ChannelsHandler mChannelsHandler;
		Client& mClient;
		Address mAddress;
		u64 mClientId;
		Datagram::ID mNextDatagramIdToSend = 0; // Identifiant du prochain datagramme à envoyer
		AckHandler mReceivedAcks;	//!< Pour détecter les duplications
		AckHandler mSentAcks;
		std::chrono::milliseconds mConnectionStartTime; // Connection start time, for connection timeout
		std::chrono::milliseconds mLastKeepAlive; // Last time this connection has been marked alive, for timeout disconnection
		static std::chrono::milliseconds sTimeout; // Timeout is same for all clients
		State mState = State::None;
#if NETWORK_INTERRUPTION
		bool mInterrupted = false; // Whether the connectivity is interrupted with this client (this client stopped sending us data)
		bool mDistantInterrupted = false; // Whether this client has its connectivity interrupted with one of its clients
#endif
		DisconnectionReason mDisconnectionReason = DisconnectionReason::None;
		std::vector<std::unique_ptr<Messages::Base>> mPendingMessages; // Stocke les messages avant que la connexion ne soit acceptée

	private:
#if NETWORK_INTERRUPTION
		void maintainConnection(bool distantNetworkInterrupted = false);
#else
		void maintainConnection();
#endif
		void onConnectionSent();
		void onConnectionReceived();
		void onConnected();
		void onDisconnectionFromOtherEnd();
		void onConnectionInterrupted();
#if NETWORK_INTERRUPTION
		// Called when this client reports having a connection interruption with one if its clients.
		void onConnectionInterruptedForwarded();
		void onConnectionResumed();
#endif
		void onConnectionLost();
		void onConnectionRefused();
		void onConnectionTimedOut();

		void onDatagramSentAcked(Datagram::ID datagramId);
		void onDatagramSentLost(Datagram::ID datagramId);
		void onDatagramReceivedLost(Datagram::ID datagramId);
		void onDataReceived(const u8* data, u16 datasize);
		void onMessageReady(std::unique_ptr<Messages::Base>&& msg);

		void fillKeepAlive(Datagram& dgram);
		void handleKeepAlive(const u8* data, const u16 datasize);

		void fillDatagramHeader(Datagram& dgram, Datagram::Type type);
		void send(const Datagram& dgram);
	};
}