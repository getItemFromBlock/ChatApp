#pragma once

#include "Core/Types.hpp"

namespace Networking::Serialization::Conversion
{
	void ToNetwork(u16 in, u16& out);
	void ToNetwork(u32 in, u32& out);
	void ToNetwork(u64 in, u64& out);
	void ToNetwork(f32 in, u32& out);
	void ToNetwork(f64 in, u64& out);
	
	void ToLocal(u16 in, u16& out);
	void ToLocal(u32 in, u32& out);
	void ToLocal(u64 in, u64& out);
	void ToLocal(u32 in, f32& out);
	void ToLocal(u64 in, f64& out);
}