#pragma once

#include <string>

#include <ImGUI/imgui.h>

#include "Resources/Texture.hpp"
#include "Core/Types.hpp"

namespace Chat
{
	class User
	{
	public:
		User() = delete;
		User(const char* name, const u64 id, const Resources::Texture* tex);

		~User();

		std::string userName;
		u64 userID = 0;
		ImVec4 userColor = ImVec4(1,1,1,1);
		const Resources::Texture* userTex;
	private:
	};

}