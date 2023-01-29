#pragma once

#include <vector>
#include "Conversion.hpp"

namespace Networking::Serialization
{
	class Serializer
	{
	public:
		Serializer() = default;

		~Serializer() = default;

		const u8* GetBuffer() const { return buffer.data(); }
		const u64 GetBufferSize() const { return buffer.size(); }

		void Write(u8 in);
		void Write(s8 in);
		void Write(u16 in);
		void Write(s16 in);
		void Write(u32 in);
		void Write(s32 in);
		void Write(u64 in);
		void Write(s64 in);
		void Write(f32 in);
		void Write(f64 in);
		void Write(u8* dataIn, u64 dataSize);
	private:
		std::vector<u8> buffer;
	};

}