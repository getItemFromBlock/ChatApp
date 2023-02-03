#include "Chat/UserManager.hpp"

using namespace Chat;

UserManager::UserManager(Resources::TextureManager& manager) : textures(manager)
{
	std::unique_ptr<User> unknownUser = std::make_unique<User>("Unknown User", 0, textures.GetDefaultTexture());
	users.emplace(0, std::move(unknownUser));
}

User* UserManager::GetUser(u64 userID)
{
	User* ptr;
	auto res = users.find(userID);
	if (res == users.end())
	{
		ptr = GetDefaultUser();
	}
	else
	{
		ptr = res->second.get();
	}
	return ptr;
}

User* Chat::UserManager::GetUserWithNetID(u64 networkID)
{
	User* result = nullptr;
	for (auto& u : users)
	{
		if (u.second->networkID == networkID)
		{
			result = u.second.get();
			break;
		}
	}
	return result;
}

User* UserManager::GetOrCreateUser(u64 userID)
{
	User* ptr;
	auto res = users.find(userID);
	if (res == users.end())
	{
		std::unique_ptr<User> tempUser = std::make_unique<User>("User", userID, textures.GetDefaultTexture());
		ptr = tempUser.get();
		users.emplace(userID, std::move(tempUser));
	}
	else
	{
		ptr = res->second.get();
	}
	return ptr;
}

User* UserManager::GetDefaultUser()
{
	return users.at(0).get();
}

const std::unordered_map<u64, std::unique_ptr<Chat::User>>& Chat::UserManager::GetAllUsers()
{
	return users;
}
