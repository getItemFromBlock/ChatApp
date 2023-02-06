#pragma once

#include <string>
#include <time.h>

#include "Core/Types.hpp"
#include "Resources/Texture.hpp"
#include "Chat/User.hpp"
#include "ActionData.hpp"

namespace Chat
{
	class ChatMessage
	{
	public:
		ChatMessage() = default;
		ChatMessage(User* s, s64 tm, u64 id = 0);

		virtual ~ChatMessage() = default;

		virtual void Draw() const = 0;

		virtual Chat::ActionData Serialize() const = 0;

		User* GetSender() const { return sender; }
		f32 GetHeight() const { return height; }
		u64 GetID() const { return messageID; }
		s64 GetTimeStamp() const { return unixTime; }
	protected:
		void DrawUser(ImVec2& pos) const;

		User* sender = nullptr;
		u64 messageID = 0;
		s64 unixTime = 0;
		char messageSTR[17] = "0000000000000000";
		std::string timestamp = "01/01/1970";
		f32 height = 0.0f;
	};

	class TextMessage : public ChatMessage
	{
	public:
		TextMessage() = default;
		TextMessage(std::string& textIn, User* userIn, s64 tm, u64 id = 0);

		virtual ~TextMessage() override = default;
		virtual void Draw() const override;
		Chat::ActionData Serialize() const override;

		static float MaxWidth;
	protected:
		std::string message;
	};

	class ImageMessage : public ChatMessage
	{
	public:
		ImageMessage() = default;
		ImageMessage(Resources::Texture* img, User* userIn, s64 tm, u64 id = 0);

		virtual ~ImageMessage() override = default;
		virtual void Draw() const override;
		Chat::ActionData Serialize() const override;
		static void SetDefaultImage(Resources::Texture* imgIn);
	protected:
		Resources::Texture* tex;
		float width = 200.0f;
		static Resources::Texture* unloadedImg;
	};

	class ConnectionMessage : public ChatMessage
	{
	public:
		ConnectionMessage() = default;
		ConnectionMessage(bool connected, User* userIn, s64 tm, u64 id = 0);

		virtual ~ConnectionMessage() override = default;
		virtual void Draw() const override;
		Chat::ActionData Serialize() const override;

	protected:
		bool connect;
	};

}