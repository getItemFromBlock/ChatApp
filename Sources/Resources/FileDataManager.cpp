#include "Resources/FileDataManager.hpp"

#include <assert.h>

using namespace Resources;

bool FileDataManager::HasPendingFiles() const
{
	return !broadcastedFiles.empty();
}

void FileDataManager::AddFileToBroadCast(const LargeFile* fileIn)
{
	broadcastedFiles.push_back(FileHolder(fileIn));
}

Chat::ActionData FileDataManager::GetNextFilePart()
{
	auto& t = broadcastedFiles.front();
	Networking::Serialization::Serializer sr;
	sr.Write(t.file->GetPath().size());
	sr.Write(reinterpret_cast<const u8*>(t.file->GetPath().c_str()), t.file->GetPath().size());
	t.file->SerializePacket(t.currentPacket, sr);
	t.currentPacket++;
	if (t.currentPacket >= t.file->GetPacketsCount()) broadcastedFiles.pop_front();
	Chat::ActionData action;
	action.type = Chat::Action::FILE_DATA;
	action.data.resize(sr.GetBufferSize());
	std::copy(sr.GetBuffer(), sr.GetBuffer() + sr.GetBufferSize(), action.data.data());
	return action;
}
