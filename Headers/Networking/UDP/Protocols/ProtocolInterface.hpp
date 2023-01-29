#pragma once

#include <vector>

#include "Networking/UDP/Datagram.hpp"
#include "Networking/NetworkSettings.hpp"
#include "Core/Types.hpp"

namespace Networking::UDP::Protocols
{
	class IProtocol
	{
	public:
		IProtocol(u8 channelId) : mChannelId(channelId) {};
		virtual ~IProtocol() = default;

		u8 channelId() const { return mChannelId; }

		virtual void queue(std::vector<uint8_t>&& msgData) = 0;
#if NETWORK_INTERRUPTION
		virtual u16 serialize(uint8_t* buffer, u16 buffersize, Datagram::ID datagramId, bool connectionInterrupted) = 0;
#else
		virtual u16 serialize(uint8_t* buffer, u16 buffersize, Datagram::ID datagramId) = 0;
#endif

		virtual void onDatagramAcked(Datagram::ID /*datagramId*/) {}
		virtual void onDatagramLost(Datagram::ID /*datagramId*/) {}

		virtual void onDataReceived(const uint8_t* data, u16 datasize) = 0;
		virtual std::vector<std::vector<uint8_t>> process() = 0;

		virtual bool isReliable() const = 0;
	private:
		u8 mChannelId;
	};
}