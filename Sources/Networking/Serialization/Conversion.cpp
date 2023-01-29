#include "Networking/Serialization/Conversion.hpp"

#ifdef _WIN32
#define NOMINMAX
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

namespace Networking::Serialization::Conversion
{
	void ToNetwork(u16 from, u16& to)
	{
		to = htons(from);
	}
	void ToNetwork(u32 from, u32& to)
	{
		to = htonl(from);
	}
	void ToNetwork(u64 from, u64& to)
	{
		to = htonll(from);
	}
	void ToNetwork(f32 from, u32& to)
	{
		to = htonf(from);
	}
	void ToNetwork(f64 from, u64& to)
	{
		to = htond(from);
	}

	void ToLocal(u16 from, u16& to)
	{
		to = ntohs(from);
	}
	void ToLocal(u32 from, u32& to)
	{
		to = ntohl(from);
	}
	void ToLocal(u64 from, u64& to)
	{
		to = ntohll(from);
	}
	void ToLocal(u32 from, f32& to)
	{
		to = ntohf(from);
	}
	void ToLocal(u64 from, f64& to)
	{
		to = ntohd(from);
	}
}