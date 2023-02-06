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

void Resources::FileDataManager::AddFileToUser(u64 userNetworkID, const LargeFile* fileIn)
{
	files[userNetworkID].push_back(UserTransferDataHolder(fileIn));
}

void Resources::FileDataManager::AddMessageToUser(u64 userNetworkID, const Chat::ChatMessage* messIn)
{
	files[userNetworkID].push_back(UserTransferDataHolder(messIn));
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

bool Resources::FileDataManager::HasUserPendingData(u64 userNetworkID)
{
	return !files[userNetworkID].empty();
}

Chat::ActionData Resources::FileDataManager::GetNextUserDataPart(u64 userNetworkID)
{
	auto& t = files[userNetworkID].front();
	Chat::ActionData action;
	if (t.object.index() == 0)
	{
		Networking::Serialization::Serializer sr;
		const LargeFile* ptr = std::get<const LargeFile*>(t.object);
		sr.Write(ptr->GetPath().size());
		sr.Write(reinterpret_cast<const u8*>(ptr->GetPath().c_str()), ptr->GetPath().size());
		ptr->SerializePacket(t.currentPacket, sr);
		t.currentPacket++;
		if (t.currentPacket >= ptr->GetPacketsCount()) files[userNetworkID].pop_front();
		action.type = Chat::Action::FILE_DATA;
		action.data.resize(sr.GetBufferSize());
		std::copy(sr.GetBuffer(), sr.GetBuffer() + sr.GetBufferSize(), action.data.data());
	}
	else
	{
		const Chat::ChatMessage* ptr = std::get<const Chat::ChatMessage*>(t.object);
		action = ptr->Serialize();
		files[userNetworkID].pop_front();
	}
	return action;
}
