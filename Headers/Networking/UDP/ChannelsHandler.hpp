#pragma once

#include <memory>
#include <vector>

#include "Datagram.hpp"
#include "Networking/UDP/Protocols/ProtocolInterface.hpp"

#define CHANNEL_UNRELIABLE 0
#define CHANNEL_RELIABLE 1

namespace Networking::UDP
{

	struct ChannelHeader
	{
		u32 channelId;
		u16 datasize;

		static constexpr u8 Size = sizeof(channelId) + sizeof(datasize);
	};

	class ChannelsHandler
	{
	public:
		ChannelsHandler();
		~ChannelsHandler();

		// Multiplexeur
		void queue(std::vector<u8>&& msgData, u32 canalIndex);
		u16 serialize(u8* buffer, u16 buffersize, Datagram::ID datagramId
#if NETWORK_INTERRUPTION
			, bool connectionInterrupted
#endif
		);

		void onDatagramAcked(Datagram::ID datagramId);
		void onDatagramLost(Datagram::ID datagramId);

		// Demultiplexeur
		void onDataReceived(const u8* data, u16 datasize);
		std::vector<std::tuple<u8 /*ChannelId*/, std::vector<u8>>> process(bool isConnected);

		template<class T>
		void registerChannel(u8 channelID)
		{
			mChannels.push_back(std::make_unique<T>(channelID));
		}

	private:
		std::vector<std::unique_ptr<Protocols::IProtocol>> mChannels;
	};
}
