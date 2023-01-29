#pragma once

#include <unordered_map>
#include <memory>
#include <vector>

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
		ChatManager(UserManager* u, Resources::TextureManager* t, u64 s, ImGui::FileBrowser* br)
			: users(u), textures(t), selfID(s), browser(br) {};

		virtual ~ChatManager() = default;

		virtual bool isHost() const = 0;

		virtual void SendChatMessage() = 0;

		virtual void Render();

		virtual void Update(f32 dt) = 0;

		void ReceiveMessage(std::unique_ptr<ChatMessage>&& mess);

	protected:
		UserManager* users;
		Resources::TextureManager* textures;
		u64 selfID = 0;
		ImGui::FileBrowser* browser = nullptr;
		std::vector<std::unique_ptr<ChatMessage>> messages;
		std::unique_ptr<ChatNetworkThread> ntwThread;
		float totalHeight = 0.0f;
		std::string currentText;
		f32 lastHeight = 0;
		TextureError lastError = TextureError::NONE;
		bool setDown = false;

		void DrawPopup();
	};

	class ClientChatManager : public ChatManager
	{
	public:
		ClientChatManager(UserManager* users, Resources::TextureManager* textures, u64 s, ImGui::FileBrowser* br);

		~ClientChatManager() override = default;

		bool isHost() const override { return false; }

		void SendChatMessage() override;

		void Render() override;

		void Update(f32 dt) override;
	private:

	};

}