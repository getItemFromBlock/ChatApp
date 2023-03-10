#pragma once

#include <unordered_map>
#include <memory>
#include <list>

#include "ChatMessage.hpp"
#include "ChatNetworkThread.hpp"

#include "UserManager.hpp"
#include "Resources/TextureManager.hpp"

namespace ImGui
{
	class FileBrowser;
}

namespace Chat
{
	class ChatManager
	{
	public:
		ChatManager() = delete;
		ChatManager(UserManager* u, Resources::TextureManager* t, u64 s, ImGui::FileBrowser* br);

		virtual ~ChatManager() = default;

		virtual bool isHost() const = 0;

		virtual void SendChatMessage() = 0;

		virtual void SendChatImage(Resources::Texture* tex) = 0;

		virtual void Update() = 0;

		void UpdateUserName();

		void UpdateUserColor();

		void UpdateUserIcon();

		virtual void Render();

		void ReceiveMessage(std::unique_ptr<ChatMessage>&& mess);

		const std::list<std::unique_ptr<ChatMessage>>& GetAllMessages();

	protected:
		UserManager* users;
		Resources::TextureManager* textures;
		u64 selfID = 0;
		ImGui::FileBrowser* browser = nullptr;
		std::list<std::unique_ptr<ChatMessage>> messages;
		std::unique_ptr<ChatNetworkThread> ntwThread;
		float totalHeight = 0.0f;
		std::string currentText;
		f32 lastHeight = 0;
		TextureError lastError = TextureError::NONE;
		bool setDown = false;
		u16 serverPort = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()) & 0xffff;
		bool isIPV6 = false;

		void DrawPopup();
	};

	class ClientChatManager : public ChatManager
	{
	public:
		ClientChatManager(UserManager* users, Resources::TextureManager* textures, u64 s, ImGui::FileBrowser* br);

		~ClientChatManager() override = default;

		bool isHost() const override { return false; }

		void SendChatMessage() override;

		void SendChatImage(Resources::Texture* tex) override;

		void Render() override;

		void Update() override;

	private:
		std::string serverAddress;
		bool rRandom = false;
		void RenderConnectionScreen();
	};

	class ServerChatManager : public ChatManager
	{
	public:
		ServerChatManager(UserManager* users, Resources::TextureManager* textures, u64 s, ImGui::FileBrowser* br);

		~ServerChatManager() override = default;

		bool isHost() const override { return true; }

		void SendChatMessage() override;

		void SendChatImage(Resources::Texture* tex) override;

		void Render() override;

		void Update() override;

	private:
	};

}