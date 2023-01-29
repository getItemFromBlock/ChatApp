#include "Chat/ChatNetworkThread.hpp"

#include <iostream>

#include "Networking/UDP/Protocols/ReliableOrdered.hpp"
#include "Networking/Errors.hpp"
#include "Networking/Serialization/Serializer.hpp"
#include "Networking/Serialization/Deserializer.hpp"
#include "Networking/Messages.hpp"
#include "Chat/ChatManager.hpp"

Chat::ChatNetworkThread::ChatNetworkThread()
{
	client.registerChannel<Networking::UDP::Protocols::ReliableOrdered>();
}

Chat::ChatNetworkThread::~ChatNetworkThread()
{
	shouldQuit.Store(true);
	t.join();
}

void Chat::ChatNetworkThread::Update(f32 dt)
{
	actions.clear();
	for (size_t i = 0; i < actionQueue.size(); i++)
	{
		actions.push_back(actionQueue[i]);
	}
	if (actions.size() == 0)
	{
		actions.push_back(ActionData(Action::PING, nullptr, 0));
	}
	actionQueue.clear();
	signal.Store(true);
}

void Chat::ChatNetworkThread::PushAction(Action type, void* data, u64 dataSize)
{
	actionQueue.push_back(ActionData(type, data, dataSize));
}

Chat::ChatClientThread::ChatClientThread(Networking::Address& addressIn)
{
	address = addressIn;
	if (!client.init(0))
	{
		std::cout << "Erreur d’initialisation du socket : " << Networking::Sockets::GetError();
		return;
	}
	client.connect(address);
	t = std::thread(&ChatClientThread::ThreadFunc, this);
}

Chat::ChatClientThread::~ChatClientThread()
{
}

bool ProcessUserUpdate(Networking::Serialization::Deserializer& dr, Chat::UserManager* users)
{
	Chat::User* user;
	u64 userID;
	u64 mTime;
	if (!dr.Read(mTime) || !dr.Read(userID))
	{
		return false;
	}
	user = users->GetOrCreateUser(userID);
	std::string tmp;
	u64 size;
	if (!dr.Read(size)) return false;
	tmp.reserve(size);
	if (!dr.Read(reinterpret_cast<u8*>(tmp.data()), size)) return false;
	if (!dr.Read(user->userColor.x) || !dr.Read(user->userColor.y) || !dr.Read(user->userColor.z)) return false;
	// TODO read and assign texture
}

void Chat::ChatClientThread::Update(f32 dt, ChatManager* manager, UserManager* users, Resources::TextureManager* textures)
{
	if (!signal.Load())
	{
		for (auto& a : actions)
		{
			std::unique_ptr<Chat::ConnectionMessage> mess;
			User* user;
			u64 userID;
			u64 mTime;
			Networking::Serialization::Deserializer dr = Networking::Serialization::Deserializer(static_cast<u8*>(a.data), a.dataSize);
			switch (a.type)
			{
			case Action::PING:
				break;
			case Action::USER_CONNECT:
				if (!dr.Read(mTime) || !dr.Read(userID))
				{
					continue;
				}
				mess = std::make_unique<Chat::ConnectionMessage>(true, users->GetUser(userID), mTime);
				manager->ReceiveMessage(std::move(mess));
				break;
			case Action::USER_DISCONNECT:
				if (!dr.Read(mTime) || !dr.Read(userID))
				{
					continue;
				}
				mess = std::make_unique<Chat::ConnectionMessage>(false, users->GetUser(userID), mTime);
				manager->ReceiveMessage(std::move(mess));
				break;
			case Action::MESSAGE_TEXT:

				break;
			case Action::MESSAGE_IMAGE:

				break;
			case Action::USER_UPDATE:

				break;
			default:
				std::cout << "Warning, Invalid action type" << std::endl;
				break;
			}
		}
		ChatNetworkThread::Update(dt);
	}
}

void Chat::ChatClientThread::ThreadFunc()
{
	while (!shouldQuit.Load())
	{
		if (signal.Load())
		{
			Networking::Serialization::Serializer sr;
			for (size_t i = 0; i < actions.size(); i++)
			{
				sr.Write(static_cast<u8>(actions[i].type));
				sr.Write(actions[i].dataSize);
				if (actions[i].dataSize != 0)
				{
					sr.Write(static_cast<u8*>(actions[i].data), actions[i].dataSize);
				}
			}
			client.sendTo(address, sr.GetBuffer(), sr.GetBufferSize(), 0);
			actions.clear();
			client.receive();
			client.processSend();
			auto v = client.poll();
			for (auto& m : v)
			{
				if (m->is<Networking::Messages::IncomingConnection>())
				{
					// should not happen on client side
				}
				else if (m->is<Networking::Messages::Connection>())
				{
					if (state == ChatNetworkState::WAITING_CONNECTION)
					{
						auto ud = m->as<Networking::Messages::Connection>();
						if (ud->result == Networking::Messages::Connection::Result::Success)
						{
							state = ChatNetworkState::CONNECTED;
						}
					}
				}
				else if (m->is<Networking::Messages::UserData>())
				{
					auto ud = m->as<Networking::Messages::UserData>();
					Networking::Serialization::Deserializer dr(ud->data.data(), ud->data.size());
					ActionData action;
					if (!dr.Read(reinterpret_cast<u8&>(action.type)) || !dr.Read(action.dataSize))
					{
						std::cout << "Warning, Corrupted message found!" << std::endl;
						continue;
					}
					if (action.dataSize != 0)
					{
						u8* tmpData = new u8[action.dataSize];
						if (!dr.Read(tmpData, action.dataSize))
						{
							std::cout << "Warning, Corrupted message found!" << std::endl;
							continue;
						}
						action.data = tmpData;
					}
					actions.push_back(std::move(action));
				}
				else if (m->is<Networking::Messages::Disconnection>())
				{
					state = ChatNetworkState::DISCONNECTED;
				}
			}
			signal.Store(true);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	client.disconnect(address);
	client.processSend();
}
