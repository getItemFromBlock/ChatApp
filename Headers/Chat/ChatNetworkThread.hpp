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

namespace Chat
{
	class ChatManager;

	enum class Action : u8
	{
		PING,
		USER_CONNECT,
		USER_DISCONNECT,
		MESSAGE_TEXT,
		MESSAGE_IMAGE,
		USER_UPDATE_NAME,
		USER_UPDATE_COLOR,
		USER_UPDATE_ICON,
	};

	enum class ChatNetworkState : u8
	{
		DISCONNECTED,
		CONNECTED,
		WAITING_CONNECTION,
	};

	class ActionData
	{
	public:
		ActionData() = default;
		ActionData(Action typeIn, u8* dataIn, u64 szIn) : type(typeIn), data(dataIn, dataIn + szIn) { }

		~ActionData() = default;

		Chat::Action type = Action::PING;
		std::vector<u8> data;
	};

	class ChatNetworkThread
	{
	public:
		ChatNetworkThread(User* selfUser);

		virtual ~ChatNetworkThread();

		void SetAddress(Networking::Address& address);

		void Update();

		virtual void TryConnect() = 0;

		void PushAction(Action type, u8* data, u64 dataSize);

		ChatNetworkState GetState() const { return state; }
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
	};

	class ChatClientThread : public ChatNetworkThread
	{
	public:
		ChatClientThread() = delete;
		ChatClientThread(User* selfUser);

		void TryConnect() override;

		~ChatClientThread() override;

		void Update(ChatManager* manager, UserManager* users, Resources::TextureManager* textures);

		void ThreadFunc();
	private:
	};

	class ChatServerThread : public ChatNetworkThread
	{
	public:
		ChatServerThread() = delete;
		ChatServerThread(User* selfUser);

		~ChatServerThread() override;

		u64 GetMessageCounter();

		void TryConnect() override;

		void Update(ChatManager* manager, UserManager* users, Resources::TextureManager* textures);

		void ThreadFunc();
	private:
		bool ProcessServerTextMessage(Networking::Serialization::Deserializer& dr, Chat::UserManager* users, Chat::ChatManager* manager);
		bool ProcessServerUserColorUpdate(Networking::Serialization::Deserializer& dr, Chat::UserManager* users);
		bool ProcessServerUserNameUpdate(Networking::Serialization::Deserializer& dr, Chat::UserManager* users);
		bool ProcessServerUserIconUpdate(Networking::Serialization::Deserializer& dr, Chat::UserManager* users, Resources::TextureManager* textures);

		std::forward_list<u64> acceptedClients;
	};

}