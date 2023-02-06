#pragma once

#include <list>
#include <unordered_map>
#include <variant>

#include "LargeFile.hpp"
#include "Chat/ActionData.hpp"
#include "Chat/ChatMessage.hpp"

namespace Resources
{
	struct FileHolder
	{
		const LargeFile* file = nullptr;
		u32 currentPacket = 0;

		FileHolder(const LargeFile* in) : file(in) {}
	};

	struct UserTransferDataHolder
	{
		std::variant<const LargeFile*, const Chat::ChatMessage*> object;
		u32 currentPacket = 0;

		UserTransferDataHolder(const LargeFile* in) : object(in) {}
		UserTransferDataHolder(const Chat::ChatMessage* in) : object(in) {}
	};

	class FileDataManager
	{
	public:
		FileDataManager() = default;

		~FileDataManager() = default;

		bool HasPendingFiles() const;
		void AddFileToBroadCast(const LargeFile* fileIn);
		void AddFileToUser(u64 userNetworkID, const LargeFile* fileIn);
		void AddMessageToUser(u64 userNetworkID, const Chat::ChatMessage* messIn);
		Chat::ActionData GetNextFilePart();
		bool HasUserPendingData(u64 userNetworkID);
		Chat::ActionData GetNextUserDataPart(u64 userNetworkID);
	private:
		std::list<FileHolder> broadcastedFiles;
		std::unordered_map<u64, std::list<UserTransferDataHolder>> files;
	};

}