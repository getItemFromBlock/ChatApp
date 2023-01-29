#include "Networking/UDP/Protocols/UnreliableOrdered.hpp"

#include <assert.h>

#include "Networking/Utils.hpp"

namespace Networking::UDP::Protocols
{
	void UnreliableOrdered::UMultiplexer::queue(std::vector<uint8_t>& msgData)
	{
		//<! S’assurer que notre message ne dépasse pas la limite auto-imposée
		assert(msgData.size() <= Packet::MaxMessageSize);
		if (msgData.size() > Packet::DataMaxSize)
		{
			u16 queuedSize = 0;
			while (queuedSize < msgData.size())
			{
				const auto fragmentSize = std::min(Packet::DataMaxSize, static_cast<u16>(msgData.size() - queuedSize));
				Packet packet;
				packet.mHeader.id = mNextId++;
				packet.mHeader.type = ((queuedSize == 0) ? Packet::Type::FirstFragment : Packet::Type::Fragment);
				packet.mHeader.size = fragmentSize;
				memcpy(packet.data(), msgData.data() + queuedSize, fragmentSize);
				mQueue.push_back(packet);
				queuedSize += fragmentSize;
			}
			mQueue.back().mHeader.type = Packet::Type::LastFragment;
			assert(queuedSize == msgData.size());
		}
		else
		{
			//!< Message n’ayant pas besoin d’être découpé
			Packet packet;
			packet.mHeader.id = mNextId++;
			packet.mHeader.type = Packet::Type::FullMessage;
			packet.mHeader.size = static_cast<u16>(msgData.size());
			memcpy(packet.data(), msgData.data(), msgData.size());
			mQueue.push_back(packet);
		}
	}

	u16 UnreliableOrdered::UMultiplexer::serialize(uint8_t* buffer, u16 buffersize, Datagram::ID)
	{
		u16 serializedSize = 0;
		for (auto packetit = mQueue.cbegin(); packetit != mQueue.cend();)
		{
			const auto& packet = *packetit;
			if (serializedSize + packet.size() > buffersize)
			{
				packetit++;
				continue; //<! Pas assez d’espace libre dans le tampon pour inclure ce paquet
			}

			memcpy(buffer, packet.buffer(), packet.size());
			serializedSize += packet.size();
			buffer += packet.size();

			packetit = mQueue.erase(packetit);
		}
		return serializedSize;
	}

	void UnreliableOrdered::UDemultiplexer::onDataReceived(const uint8_t* data, u16 datasize)
	{
		//<! Extraire les paquets du tampon
		u16 processedDataSize = 0;
		while (processedDataSize < datasize)
		{
			const Packet* pckt = reinterpret_cast<const Packet*>(data);
			if (processedDataSize + pckt->size() > datasize || pckt->datasize() > Packet::DataMaxSize)
			{
				//<! Paquet ou tampon mal formé, on interrompt
				return;
			}
			onPacketReceived(pckt);
			processedDataSize += pckt->size();
			data += pckt->size();
		}
	}

	void UnreliableOrdered::UDemultiplexer::onPacketReceived(const Packet* pckt)
	{
		if (!Utils::IsSequenceNewer(pckt->id(), mLastProcessed))
			return; //<! Paquet trop vieux

		if (mPendingQueue.empty() || Utils::IsSequenceNewer(pckt->id(), mPendingQueue.back().id()))
		{
			mPendingQueue.push_back(*pckt);
		}
		else
		{
			//!< Find the first iterator with an id equals to or newer than our packet, we must place the packet before that one
			auto insertLocation = std::find_if(mPendingQueue.cbegin(), mPendingQueue.cend(), [&pckt](const Packet& p) { return p.id() == pckt->id() || Utils::IsSequenceNewer(p.id(), pckt->id()); });
			//!< Make sure we don't insert a packet received multiple times
			if (insertLocation->id() != pckt->id())
			{
				mPendingQueue.insert(insertLocation, *pckt);
			}
		}
	}

	std::vector<u8> UnreliableOrdered::UDemultiplexer::getFullMessage(std::vector<Packet>::const_iterator& itPacket, std::vector<Packet>::const_iterator& itEnd) const
	{
		std::vector<u8> msg(itPacket->data(), itPacket->data() + itPacket->datasize());
		auto expectedPacketId = itPacket->id();
		++itPacket;
		++expectedPacketId;
		while (itPacket != itEnd && itPacket->id() == expectedPacketId)
		{
			if (itPacket->type() == Packet::Type::LastFragment)
			{
				//!< Last fragment reached, the message is full
				msg.insert(msg.cend(), itPacket->data(), itPacket->data() + itPacket->datasize());
				return msg;
			}
			else if (itPacket->type() != Packet::Type::Fragment)
			{
				//!< If we reach this, we likely recieved a malformed packet / hack attempt
				msg.clear();
				return msg;
			}

			msg.insert(msg.cend(), itPacket->data(), itPacket->data() + itPacket->datasize());
			++itPacket;
			++expectedPacketId;
		}
		msg.clear();
		return msg;
	}

	std::vector<std::vector<uint8_t>> UnreliableOrdered::UDemultiplexer::process()
	{
		if (mPendingQueue.size() > 128) // queue too big, something went wrong
		{
			mPendingQueue.clear();
		}

		std::vector<std::vector<uint8_t>> messagesReady;
		auto itPacket = mPendingQueue.cbegin();
		auto itEnd = mPendingQueue.cend();
		std::vector<Packet>::const_iterator newestProcessedPacket;
		//!< Our queue is ordered, just go through and reassemble packets
		while (itPacket != itEnd)
		{
			if (itPacket->type() == Packet::Type::FullMessage)
			{
				//!< Full packet, just take it
				std::vector<u8> msg(itPacket->data(), itPacket->data() + itPacket->datasize());
				messagesReady.push_back(std::move(msg));
				newestProcessedPacket = itPacket;
				++itPacket;
			}
			else if (itPacket->type() == Packet::Type::FirstFragment)
			{
				//!< Check if the message is ready (fully received)
				std::vector<u8> msg = getFullMessage(itPacket, itEnd);
				if (!msg.empty())
				{
					//!< We do have a message
					messagesReady.push_back(std::move(msg));
					newestProcessedPacket = itPacket;
					//!< Move iterator after the last packet of the message
					++itPacket;
				}
			}
			else
			{
				++itPacket;
			}
		}

		//!< Remove every processed and partial packets until the last one processed included
		if (!messagesReady.empty())
		{
			mLastProcessed = newestProcessedPacket->id();
			mPendingQueue.erase(mPendingQueue.cbegin(), std::next(newestProcessedPacket));
		}

		return messagesReady;
	}

	void UnreliableOrdered::queue(std::vector<uint8_t>&& messageData)
	{
		multiplexer.queue(messageData);
	}

#if NETWORK_INTERRUPTION
	u16 UnreliableOrdered::serialize(uint8_t* buffer, u16 buffersize, Datagram::ID, bool)
#else
	u16 UnreliableOrdered::serialize(uint8_t* buffer, u16 buffersize, Datagram::ID)
#endif
	{
		return multiplexer.serialize(buffer, buffersize);
	}

	void UnreliableOrdered::onDataReceived(const uint8_t* data, u16 datasize)
	{
		demultiplexer.onDataReceived(data, datasize);
	}

	std::vector<std::vector<uint8_t>> UnreliableOrdered::process()
	{
		return demultiplexer.process();
	}
}