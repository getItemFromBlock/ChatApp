#include "Resources/LargeFile.hpp"

using namespace Resources;

LargeFile::LargeFile()
{
}

LargeFile::~LargeFile()
{
	if (FileData)
	{
		delete[] FileData;
	}
}

bool LargeFile::PreLoad(Networking::Serialization::Deserializer& dr, const std::string& pathIn)
{
	if (FileData)
	{
		delete[] FileData;
		FileData = nullptr;
	}
	if (complete)
	{
		complete = false;
		receivedParts.clear();
	}
	path = pathIn;
	u64 tmpSize;
	if (!dr.Read(tmpSize)) return false;
	fileType.resize(tmpSize);
	if (!dr.Read(reinterpret_cast<u8*>(fileType.data()), tmpSize)) return false;
	if (!dr.Read(dataSize)) return false;
	u32 pkCount = GetPacketsCount();
	receivedParts.resize(pkCount, false);
	FileData = new u8[dataSize];
	complete = false;
	return true;
}

bool LargeFile::AcceptPacket(Networking::Serialization::Deserializer& dr)
{
	u32 packetIndex;
	u16 packetSize;
	if (!dr.Read(packetIndex) || !dr.Read(packetSize)) return false;
	if (packetIndex >= GetPacketsCount()) return false;
	bool isLast = (packetIndex + 1) == GetPacketsCount();
	if (packetSize != (isLast ? GetLastPacketSize() : 0x8000)) return false;
	u64 delta = (u64)packetIndex << 15;
	if (!dr.Read(FileData + delta, packetSize)) return false;
	receivedParts[packetIndex] = true;
	for (u32 i = 0; i < GetPacketsCount(); i++)
	{
		if (!receivedParts[i]) return true;
	}
	complete = true;
	return true;
}

bool Resources::LargeFile::SerializePacket(u32 packetIndex, Networking::Serialization::Serializer& sr) const
{
	if (packetIndex >= GetPacketsCount()) return false;
	sr.Write(packetIndex);
	u16 packetSize = (packetIndex + 1) == GetPacketsCount() ? GetLastPacketSize() : (u32)0x8000;
	sr.Write(packetSize);
	u64 delta = (u64)packetIndex << 15;
	sr.Write(FileData + delta, packetSize);
	return true;
}

bool Resources::LargeFile::SerializeFile(Networking::Serialization::Serializer& sr) const
{
	sr.Write(path.size());
	sr.Write(reinterpret_cast<const u8*>(path.data()), path.size());
	sr.Write(fileType.size());
	sr.Write(reinterpret_cast<const u8*>(fileType.data()), fileType.size());
	sr.Write(dataSize);
	return true;
}

u32 Resources::LargeFile::GetPacketsCount() const
{
	return static_cast<u32>((dataSize >> 15) + ((dataSize & 0x7fff) != 0));
}

u32 Resources::LargeFile::GetLastPacketSize() const
{
	u32 result = dataSize & 0x7fff;
	return result == 0 ? 0x8000 : result;
}

float Resources::LargeFile::GetLoadingCompletion() const
{
	if (receivedParts.size() == 0) return 1.0f;
	u32 total = 0;
	for (u32 i = 0; i < receivedParts.size(); i++) if (receivedParts[i]) total++;
	return total * 1.0f / receivedParts.size();
}
