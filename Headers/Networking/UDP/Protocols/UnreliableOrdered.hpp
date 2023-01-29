#pragma once

#include "ProtocolInterface.hpp"
#include "Packet.hpp"

namespace Networking::UDP::Protocols
{
	class UnreliableOrdered : public IProtocol
	{
	public:
		UnreliableOrdered(u8 channelId) : IProtocol(channelId) {}
		~UnreliableOrdered() override = default;

		void queue(std::vector<uint8_t>&& messageData) override;
#if NETWORK_INTERRUPTION
		u16 serialize(uint8_t* buffer, u16 buffersize, Datagram::ID, bool connectionInterrupted) override;
#else
		u16 serialize(uint8_t* buffer, u16 buffersize, Datagram::ID) override;
#endif

		void onDataReceived(const uint8_t* data, u16 datasize) override;
		std::vector<std::vector<uint8_t>> process() override;

		bool isReliable() const override { return false; }
	private:
		class UMultiplexer
		{
		public:
			UMultiplexer() = default;
			~UMultiplexer() = default;

			void queue(std::vector<uint8_t>& messageData);
			u16 serialize(uint8_t* buffer, u16 buffersize, Datagram::ID = 0);
		private:
			std::vector<Packet> mQueue;
			Packet::ID mNextId = 0;
		};

		class UDemultiplexer
		{
		public:
			UDemultiplexer() = default;
			~UDemultiplexer() = default;

			void onDataReceived(const uint8_t* data, u16 datasize);
			std::vector<std::vector<uint8_t>> process();

		private:
			void onPacketReceived(const Packet* pckt);

			std::vector<Packet> mPendingQueue;
			Packet::ID mLastProcessed = std::numeric_limits<Packet::ID>::max();
			std::vector<u8> getFullMessage(std::vector<Packet>::const_iterator& itPacket, std::vector<Packet>::const_iterator& itEnd) const;
		};
		UMultiplexer multiplexer;
		UDemultiplexer demultiplexer;
	};
}