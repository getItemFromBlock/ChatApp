#pragma once

#include <string>
#include <vector>

#include "Core/Types.hpp"
#include "Core/Signal.hpp"
#include "Networking/Serialization/Serializer.hpp"
#include "Networking/Serialization/Deserializer.hpp"

namespace Resources
{
	class LargeFile
	{
	public:
		LargeFile();

		virtual ~LargeFile();

		virtual bool PreLoad(Networking::Serialization::Deserializer& dr, const std::string& path);
		virtual bool AcceptPacket(Networking::Serialization::Deserializer& dr);
		virtual bool SerializePacket(u32 packetIndex, Networking::Serialization::Serializer& sr) const;
		virtual bool SerializeFile(Networking::Serialization::Serializer& sr) const;
		bool IsComplete() const { return complete; }
		u32 GetPacketsCount() const;
		u32 GetLastPacketSize() const;
		const std::string& GetPath() const { return path; }
		const std::string& GetFileType() const { return fileType; }
	protected:
		u8* FileData = nullptr;
		u64 dataSize = 0;
		bool complete = false;
		std::string fileType;
		std::string path;
		std::vector<bool> receivedParts;
	};

}
