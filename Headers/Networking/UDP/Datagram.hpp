#pragma once

#include <cstdint>
#include <array>

#include "Core/Types.hpp"

namespace Networking::UDP
{
	struct Datagram
	{
		using ID = u16;
		enum class Type : u8 {
			ConnectedData,
			KeepAlive,
			Disconnection,
		};
		struct Header
		{
			ID id;
			ID ack;
			u64 previousAcks;
			Type type;
		};
		static constexpr u16 BufferMaxSize = 1400;
		static constexpr u16 HeaderSize = sizeof(Header);
		static constexpr u16 DataMaxSize = BufferMaxSize - HeaderSize;

		Header header;
		std::array<u8, DataMaxSize> data;

		u16 datasize = 0;
		u16 size() const { return HeaderSize + datasize; }
	};
}
