#pragma once

#include <cstdint>
#include <chrono>

namespace Networking::Utils
{

	std::chrono::milliseconds Now();
	bool IsSequenceNewer(uint16_t sNew, uint16_t sLast);
	uint16_t SequenceDiff(uint16_t sNew, uint16_t sLast);

	inline void SetBit(uint64_t& bitfield, uint8_t n);
	inline void UnsetBit(uint64_t& bitfield, uint8_t n);
	inline bool HasBit(uint64_t bitfield, uint8_t n);

	template<class INTEGER>
	struct Bit {};
	template<>
	struct Bit<uint64_t> {
		static constexpr uint64_t Right = 0b0000000000000000000000000000000000000000000000000000000000000001;
		// extra 0's in case of a little endian architecture
	};
}

#include "Networking/Utils.inl"