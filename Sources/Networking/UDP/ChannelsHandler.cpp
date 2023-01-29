#include "Networking/UDP/ChannelsHandler.hpp"

#include "Networking/UDP/Protocols/UnreliableOrdered.hpp"
#include "Networking/UDP/Protocols/ReliableOrdered.hpp"
#include <assert.h>
#include <iostream>

namespace Networking::UDP
{
	ChannelsHandler::ChannelsHandler()
	{
	}

	ChannelsHandler::~ChannelsHandler()
	{
	}

	void ChannelsHandler::onDatagramAcked(Datagram::ID datagramId)
	{
		for (auto& protocol : mChannels)
		{
			protocol->onDatagramAcked(datagramId);
		}
	}

	void ChannelsHandler::onDatagramLost(Datagram::ID datagramId)
	{
		for (auto& protocol : mChannels)
		{
			protocol->onDatagramLost(datagramId);
		}
	}

	std::vector<std::tuple<u8, std::vector<u8>>> ChannelsHandler::process(bool isConnected)
	{
		std::vector<std::tuple<u8, std::vector<u8>>> messages;
		for (auto& channel : mChannels)
		{
			std::vector<std::vector<u8>> protocolMessages = channel->process();
			// If we're not connected, ignore and discard unreliable messages
			if (!protocolMessages.empty() && (channel->isReliable() || isConnected))
			{
				messages.reserve(messages.size() + protocolMessages.size());
				for (auto&& msg : protocolMessages)
				{
					messages.push_back(std::make_tuple(channel->channelId(), std::move(msg)));
				}
			}
		}
		return messages;
	}

	void ChannelsHandler::queue(std::vector<uint8_t>&& msgData, uint32_t canalIndex)
	{
		assert(canalIndex < mChannels.size());
		mChannels[canalIndex]->queue(std::move(msgData));
	}

	u16 ChannelsHandler::serialize(u8* buffer, u16 buffersize, Datagram::ID datagramId
#if NETWORK_INTERRUPTION
		, bool connectionInterrupted
#endif
	)
	{
		u16 remainingBuffersize = buffersize;
		for (u32 protocolid = 0; protocolid < mChannels.size(); ++protocolid)
		{
			Protocols::IProtocol* protocol = mChannels[protocolid].get();

			u8* const channelHeaderStart = buffer;
			u8* const channelDataStart = buffer + ChannelHeader::Size;
			const u16 channelAvailableSize = remainingBuffersize - ChannelHeader::Size;

			const u16 serializedData = protocol->serialize(channelDataStart, channelAvailableSize, datagramId
#if NETWORK_INTERRUPTION
				, connectionInterrupted
#endif
			);
			assert(serializedData <= channelAvailableSize);
			if (serializedData)
			{
				// Le canal a sérialisé des données pour l’envoi, ajoutons l’en-tête de canal
				ChannelHeader* const channelHeader = reinterpret_cast<ChannelHeader*>(channelHeaderStart);
				channelHeader->channelId = protocolid;
				channelHeader->datasize = static_cast<u32>(serializedData);

				const u16 channelTotalSize = serializedData + ChannelHeader::Size;
				buffer += channelTotalSize;
				remainingBuffersize -= channelTotalSize;
			}
		}
		return buffersize - remainingBuffersize;
	}

	void ChannelsHandler::onDataReceived(const u8* data, u16 datasize)
	{
		u16 processedData = 0;
		while (processedData < datasize)
		{
			const ChannelHeader* channelHeader = reinterpret_cast<const ChannelHeader*>(data);
			if (processedData + channelHeader->datasize > datasize || channelHeader->datasize > Datagram::DataMaxSize)
			{
				// Tampon mal formé
				return;
			}
			if (channelHeader->channelId >= mChannels.size())
			{
				std::cout << "Error: channel " << channelHeader->channelId << "does not exist" << std::endl;
				return;
			}
			mChannels[channelHeader->channelId]->onDataReceived(data + ChannelHeader::Size, channelHeader->datasize);
			const u16 channelTotalSize = channelHeader->datasize + ChannelHeader::Size;
			data += channelTotalSize;
			processedData += channelTotalSize;
		}
	}

}