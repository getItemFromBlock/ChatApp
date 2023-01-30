#pragma once

#include <thread>
#include <vector>

#include "Networking/Address.hpp"
#include "Networking/UDP/Client.hpp"
#include "Core/Types.hpp"
#include "Core/Signal.hpp"
#include "ChatMessage.hpp"
#include "UserManager.hpp"
#include "Resources/TextureManager.hpp"

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
		ActionData(Action typeIn, void* dataIn, u64 szIn) : type(typeIn), data(dataIn), dataSize(szIn) { }

		Chat::Action type = Action::PING;
		u64 dataSize = 0;
		void* data = nullptr;
	};

	class ChatNetworkThread
	{
	public:
		ChatNetworkThread();

		virtual ~ChatNetworkThread();

		void Update(f32 dt);

		void PushAction(Action type, void* data, u64 dataSize);

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
		ChatNetworkState state = ChatNetworkState::DISCONNECTED;
	};

	class ChatClientThread : public ChatNetworkThread
	{
	public:
		ChatClientThread() = delete;
		ChatClientThread(Networking::Address& address);

		virtual ~ChatClientThread() override;

		void Update(f32 dt, ChatManager* manager, UserManager* users, Resources::TextureManager* textures);

		void Connect();

		void ThreadFunc();
	private:
	};

	class ChatServerThread : public ChatNetworkThread
	{
	public:
		ChatServerThread() = delete;
		ChatServerThread(Networking::Address& address);

		virtual ~ChatServerThread() override;

		void Update(f32 dt, ChatManager* manager, UserManager* users, Resources::TextureManager* textures);

		void ThreadFunc();
	private:
	};

}