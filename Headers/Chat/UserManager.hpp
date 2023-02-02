#pragma once

#include <unordered_map>
#include <memory>

#include "User.hpp"
#include "Resources/TextureManager.hpp"

namespace Chat
{
	class UserManager
	{
	public:
		UserManager() = delete;
		UserManager(Resources::TextureManager& manager);

		~UserManager() = default;

		User* GetUser(u64 userID);

		User* GetUserWithNetID(u64 networkID);

		User* GetOrCreateUser(u64 userID);

		User* GetDefaultUser();

	private:
		std::unordered_map<u64, std::unique_ptr<Chat::User>> users;
		Resources::TextureManager& textures;
	};

}