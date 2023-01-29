#pragma once

#include <set>

#include "ProtocolInterface.hpp"
#include "Packet.hpp"

namespace Networking::UDP::Protocols
{
	class ReliableOrdered : public IProtocol
	{
	public:
		ReliableOrdered(u8 channelId) : IProtocol(channelId) {}
		~ReliableOrdered() override = default;

		void queue(std::vector<u8>&& msgData) override;
#if NETWORK_INTERRUPTION
		u16 serialize(u8* buffer, u16 buffersize, Datagram::ID datagramId, bool connectionInterrupted) override;
#else
		u16 serialize(u8* buffer, u16 buffersize, Datagram::ID datagramId) override;
#endif

		void onDatagramAcked(Datagram::ID datagramId) override;
		void onDatagramLost(Datagram::ID datagramId) override;

		void onDataReceived(const u8* data, u16 datasize) override;
		std::vector<std::vector<u8>> process() override;

		bool isReliable() const override { return true; }
	private:
		class RMultiplexer
		{
		public:
			RMultiplexer() = default;
			~RMultiplexer() = default;

			void queue(std::vector<u8>& msgData);
#if NETWORK_INTERRUPTION
			u16 serialize(u8* buffer, u16 buffersize, Datagram::ID datagramId, bool connectionInterrupted);
#else
			u16 serialize(u8* buffer, u16 buffersize, Datagram::ID datagramId);
#endif

			void onDatagramAcked(Datagram::ID datagramId);
			void onDatagramLost(Datagram::ID datagramId);
		private:
			class ReliablePacket
			{
			public:
				Packet& packet() { return mPacket; }
				const Packet& packet() const { return mPacket; }

				void onSent(Datagram::ID datagramId) { mDatagramsIncluding.insert(datagramId); mShouldSend = false; }
				bool isIncludedIn(Datagram::ID datagramId) const { return mDatagramsIncluding.find(datagramId) != mDatagramsIncluding.cend(); }
				void resend() { mShouldSend = true; }
				bool shouldSend() { return mShouldSend; }
			private:
				Packet mPacket;
				std::set<Datagram::ID> mDatagramsIncluding;
				bool mShouldSend = true;
			};
			std::vector<ReliablePacket> mQueue;
			Packet::ID mNextId = 0;
			Packet::ID mFirstAllowedPacket = 0;
		};

		class RDemultiplexer
		{
		public:
			RDemultiplexer() = default;
			~RDemultiplexer() = default;

			void onDataReceived(const u8* data, u16 datasize);
			std::vector<std::vector<u8>> process();

			static constexpr size_t QueueSize = 2 * Packet::MaxPacketsPerMessage;
		private:
			void onPacketReceived(const Packet* pckt);

			std::array<Packet, QueueSize> mPendingQueue;
			Packet::ID mLastProcessed = std::numeric_limits<Packet::ID>::max();
			bool isMessageFull(size_t index, Networking::UDP::Datagram::ID packetID, const size_t& startIndexOffset) const;
		};
		RMultiplexer multiplexer;
		RDemultiplexer demultiplexer;
	};
}