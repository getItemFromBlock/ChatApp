#pragma once

#include <string>

#include <ImGUI/imgui.h>

#include "Resources/Texture.hpp"
#include "Core/Types.hpp"
#include "Networking/Address.hpp"

namespace Chat
{
	class User
	{
	public:
		User() = delete;
		User(const char* name, u64 id, const Resources::Texture* tex);

		~User();

		std::string userName;
		u64 userID = 0;
		u64 networkID = -1;
		s64 lastActivity = 0;
		bool isConnected = true;
		Maths::Vec3 userColor = Maths::Vec3(1.0f);
		Networking::Address clientAddress;
		const Resources::Texture* userTex;
	private:
	};

}