#pragma once

#include <list>

#include "LargeFile.hpp"
#include "Chat/ActionData.hpp"

namespace Resources
{
	struct FileHolder
	{
		const LargeFile* file = nullptr;
		u32 currentPacket = 0;

		FileHolder(const LargeFile* in) : file(in) {}
	};

	class FileDataManager
	{
	public:
		FileDataManager() = default;

		~FileDataManager() = default;

		bool HasPendingFiles() const;
		void AddFileToBroadCast(const LargeFile* fileIn);
		Chat::ActionData GetNextFilePart();
	private:
		std::list<FileHolder> broadcastedFiles;
	};

}