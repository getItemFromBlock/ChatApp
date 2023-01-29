#pragma once

#include <string>
#include <unordered_map>

#include "Chat/User.hpp"

namespace Resources
{
	class SaveFile
	{
	public:
		SaveFile() = default;
		SaveFile(const char* pathIn) : path(pathIn) {};

		~SaveFile() = default;

		void LoadData(std::unordered_map<u64, Chat::User>& userList);
		void SaveData(std::unordered_map<u64, Chat::User>& userList);

	private:
		std::string path;
	};

}