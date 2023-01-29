#pragma once

#include <string>
#include <time.h>

#include "Core/Types.hpp"
#include "Resources/Texture.hpp"
#include "Chat/User.hpp"

namespace Chat
{
	class ChatMessage
	{
	public:
		ChatMessage() = default;
		ChatMessage(User* s, u64 tm);

		virtual ~ChatMessage() = default;

		virtual void Draw() = 0;

		User* GetSender() { return sender; }
		f32 GetHeight() { return height; }
		u64 GetID() { return messageID; }
	protected:
		void DrawUser(ImVec2& pos);

		User* sender = nullptr;
		u64 messageID = 0;
		char messageSTR[17] = "0000000000000000";
		std::string timestamp = "01/01/1970";
		f32 height = 0.0f;

		static u64 index;
	};

	class TextMessage : public ChatMessage
	{
	public:
		TextMessage() = default;
		TextMessage(std::string& textIn, User* userIn, u64 tm);

		virtual ~TextMessage() override = default;
		virtual void Draw() override;

		static float MaxWidth;
	protected:
		std::string message;
	};

	class ImageMessage : public ChatMessage
	{
	public:
		ImageMessage() = default;
		ImageMessage(Resources::Texture* img, User* userIn, u64 tm);

		virtual ~ImageMessage() override = default;
		virtual void Draw() override;

	protected:
		Resources::Texture* tex;
		float width = 200.0f;
	};

	class ConnectionMessage : public ChatMessage
	{
	public:
		ConnectionMessage() = default;
		ConnectionMessage(bool connected, User* userIn, u64 tm);

		virtual ~ConnectionMessage() override = default;
		virtual void Draw() override;

	protected:
		bool connect;
	};

}