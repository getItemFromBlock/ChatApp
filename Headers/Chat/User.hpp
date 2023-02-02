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
		User(const char* name, u64 id, const Resources::Texture* tex);

		~User();

		std::string userName;
		u64 userID = 0;
		u64 networkID = 0;
		Maths::Vec3 userColor = Maths::Vec3(1.0f);
		const Resources::Texture* userTex;
	private:
	};

}