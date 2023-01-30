#include "Networking/Serialization/Serializer.hpp"

using namespace Networking::Serialization;

void Serializer::Write(u8 in)
{
	buffer.push_back(in);
}

void Serializer::Write(s8 in)
{
	buffer.push_back(static_cast<u8>(in));
}

void Serializer::Write(u16 in)
{
	u16 tmp;
	Conversion::ToNetwork(in, tmp);
	for (u8 i = 0; i < sizeof(tmp); i++)
	{
		buffer.push_back((tmp >> (i*8) & 0xff));
	}
}

void Serializer::Write(s16 in)
{
	Write(static_cast<u16>(in));
}

void Serializer::Write(u32 in)
{
	u32 tmp;
	Conversion::ToNetwork(in, tmp);
	for (u8 i = 0; i < sizeof(tmp); i++)
	{
		buffer.push_back((tmp >> (i * 8) & 0xff));
	}
}

void Serializer::Write(s32 in)
{
	Write(static_cast<u32>(in));
}

void Serializer::Write(u64 in)
{
	u64 tmp;
	Conversion::ToNetwork(in, tmp);
	for (u8 i = 0; i < sizeof(tmp); i++)
	{
		buffer.push_back((tmp >> (i * 8) & 0xff));
	}
}

void Serializer::Write(s64 in)
{
	Write(static_cast<u64>(in));
}

void Serializer::Write(f32 in)
{
	u32 tmp;
	Conversion::ToNetwork(in, tmp);
	for (u8 i = 0; i < sizeof(tmp); i++)
	{
		buffer.push_back((tmp >> (i * 8) & 0xff));
	}
}

void Serializer::Write(f64 in)
{
	u64 tmp;
	Conversion::ToNetwork(in, tmp);
	for (u8 i = 0; i < sizeof(tmp); i++)
	{
		buffer.push_back((tmp >> (i * 8) & 0xff));
	}
}

void Networking::Serialization::Serializer::Write(const u8* dataIn, u64 dataSize)
{
	buffer.resize(buffer.size() + dataSize);
	std::copy(dataIn, dataIn + dataSize, buffer.data() + (buffer.size() - dataSize));
}
