#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filesystem>

#include "Networking/Network.hpp"
#include "Chat/ChatManager.hpp"
#include "Resources/TextureManager.hpp"
#include "Chat/UserManager.hpp"

namespace ImGui
{
	class FileBrowser;
}

namespace Core
{
	class App
	{
	public:
		App();

		~App();

		int Init();
		void Run();

	private:
		std::unique_ptr<ImGui::FileBrowser> fileDialog;

		GLFWwindow* window = nullptr;
		GLFWimage* windowIcon = nullptr;
		Networking::Network network;
		u64 selfID = 0;
		std::unique_ptr<Chat::UserManager> users;
		std::unique_ptr<Chat::ChatManager> manager;
		std::unique_ptr<Resources::TextureManager> textures;
		Chat::User* selfUser = nullptr;
		std::string tmpUserName;
		Maths::Vec3 tmpUserColor;
		const Resources::Texture* tmpTexture = nullptr;
		TextureError lastError = TextureError::NONE;
		bool userSettings = false;
	};

}