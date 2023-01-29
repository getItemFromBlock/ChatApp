#pragma once

#include <vector>
#include "Conversion.hpp"

namespace Networking::Serialization
{
	class Deserializer
	{
	public:
		Deserializer(const u8* data, u64 dataSize) : buffer(data), bufferSize(dataSize) {}

		~Deserializer() = default;

		const u64 CursorPos() const { return cPos; }

		bool Read(u8& in);
		bool Read(s8& in);
		bool Read(u16& in);
		bool Read(s16& in);
		bool Read(u32& in);
		bool Read(s32& in);
		bool Read(u64& in);
		bool Read(s64& in);
		bool Read(f32& in);
		bool Read(f64& in);
		bool Read(u8* dataIn, u64 dataSize);
	private:
		const u8* buffer;
		const u64 bufferSize;
		u64 cPos = 0;
	};

}