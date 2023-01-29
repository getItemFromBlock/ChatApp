#pragma once

#include <vector>

#include "Networking/UDP/Datagram.hpp"
#include "Networking/UDP/ChannelsHandler.hpp"

namespace Networking::UDP::Protocols
{
	struct Packet
	{
		using ID = u16;
		enum class Type : u8
		{
			FullMessage,
			FirstFragment,
			Fragment,
			LastFragment,
		};
		struct Header
		{
			ID id = 0;
			u16 size = 0;
			Type type = Type::Fragment;
		};
		static constexpr u16 PacketMaxSize = Datagram::DataMaxSize - ChannelHeader::Size;
		static constexpr u16 HeaderSize = sizeof(Header);
		static constexpr u16 DataMaxSize = PacketMaxSize - HeaderSize;
		static constexpr u8 MaxPacketsPerMessage = 32;
		static constexpr u16 MaxMessageSize = MaxPacketsPerMessage * DataMaxSize;

		u16 size() const { return mHeader.size + HeaderSize; }
		const u8* buffer() const { return reinterpret_cast<const u8*>(this); }
		const u16 datasize() const { return mHeader.size; }
		const u16 id() const { return mHeader.id; }
		const Type type() const { return mHeader.type; }
		uint8_t* data() { return mData.data(); }
		const u8* data() const { return mData.data(); }
		Header mHeader;
		std::array<u8, DataMaxSize> mData;
	};
}