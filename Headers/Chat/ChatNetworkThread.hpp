#pragma once

#include <thread>
#include <vector>
#include <forward_list>

#include "Networking/Address.hpp"
#include "Networking/UDP/Client.hpp"
#include "Core/Types.hpp"
#include "Core/Signal.hpp"
#include "ChatMessage.hpp"
#include "UserManager.hpp"
#include "Resources/TextureManager.hpp"
#include "Networking/Serialization/Serializer.hpp"
#include "Networking/Serialization/Deserializer.hpp"
#include "ActionData.hpp"
#include "Resources/FileDataManager.hpp"

namespace Chat
{
	class ChatManager;

	enum class ChatNetworkState : u8
	{
		DISCONNECTED,
		CONNECTED,
		WAITING_CONNECTION,
		CONNECTION_LOST,
	};

	class ChatNetworkThread
	{
	public:
		ChatNetworkThread(User* selfUser, ChatManager* manager, UserManager* users, Resources::TextureManager* textures);

		virtual ~ChatNetworkThread();

		void SetAddress(Networking::Address& address);

		void Update();

		virtual void TryConnect() = 0;

		void PushAction(Action type, const u8* data, u64 dataSize);
		void PushAction(ActionData&& action);

		Chat::ActionData SendUserColor(Chat::User* user);
		Chat::ActionData SendUserName(Chat::User* user);
		Chat::ActionData SendUserIcon(Chat::User* user);

		ChatNetworkState GetState() const { return state; }
		void ResetState() { state = ChatNetworkState::DISCONNECTED; }
		const char* GetLastError() { return lastError; }
	protected:
		std::thread t;
		Networking::Address address;
		Networking::UDP::Client client;
		std::vector<ActionData> actionQueue;
		std::vector<ActionData> actions;
		Core::Signal signal = Core::Signal(false);
		Core::Signal connect = Core::Signal(false);
		Core::Signal shouldQuit = Core::Signal(false);
		User* self = nullptr;
		ChatNetworkState state = ChatNetworkState::DISCONNECTED;
		const char* lastError = "Unknown error";
		ChatManager* manager = nullptr;
		UserManager* users = nullptr;
		Resources::TextureManager* textures = nullptr;
		Resources::FileDataManager files;
		bool clientStallBool = false;
	};

	class ChatClientThread : public ChatNetworkThread
	{
	public:
		ChatClientThread() = delete;
		ChatClientThread(User* selfUser, ChatManager* manager, UserManager* users, Resources::TextureManager* textures);

		void TryConnect() override;

		~ChatClientThread() override;

		void Update();

		void ThreadFunc();
	private:
		bool ProcessUserNameUpdate(Networking::Serialization::Deserializer& dr);
		bool ProcessUserColorUpdate(Networking::Serialization::Deserializer& dr);
		bool ProcessUserIconUpdate(Networking::Serialization::Deserializer& dr);
		bool ProcessTextMessage(Networking::Serialization::Deserializer& dr);
		bool ProcessImageMessage(Networking::Serialization::Deserializer& dr);
		bool ProcessFilePart(Networking::Serialization::Deserializer& dr);
	};

	class ChatServerThread : public ChatNetworkThread
	{
	public:
		ChatServerThread() = delete;
		ChatServerThread(User* selfUser, ChatManager* manager, UserManager* users, Resources::TextureManager* textures);

		~ChatServerThread() override;

		u64 GetMessageCounter();

		void TryConnect() override;

		void Update();

		void ThreadFunc();
	private:
		bool ProcessServerTextMessage(Networking::Serialization::Deserializer& dr);
		bool ProcessServerImageMessage(Networking::Serialization::Deserializer& dr);
		bool ProcessServerUserColorUpdate(Networking::Serialization::Deserializer& dr);
		bool ProcessServerUserNameUpdate(Networking::Serialization::Deserializer& dr);
		bool ProcessServerUserIconUpdate(Networking::Serialization::Deserializer& dr);
		bool ProcessServerUserDisconnection(Networking::Serialization::Deserializer& dr);
		bool ProcessServerUserConnection(const Networking::Address& clientIn, u64 clientNetworkID);
		bool ProcessServerFilePart(Networking::Serialization::Deserializer& dr);

		std::forward_list<u64> acceptedClients;
	};

}