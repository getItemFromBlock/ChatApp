#include "Chat/ChatNetworkThread.hpp"

#include <iostream>

#include "Networking/UDP/Protocols/ReliableOrdered.hpp"
#include "Networking/Errors.hpp"
#include "Networking/Messages.hpp"
#include "Chat/ChatManager.hpp"

Chat::ChatNetworkThread::ChatNetworkThread(User* selfUser) : self(selfUser)
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

Chat::ChatClientThread::ChatClientThread(Networking::Address& addressIn, User* selfUser) : ChatNetworkThread(selfUser)
{
	address = addressIn;
	if (!client.init(0))
	{
		std::cout << "Erreur d’initialisation du socket : " << Networking::Sockets::GetError();
		return;
	}
	t = std::thread(&ChatClientThread::ThreadFunc, this);
}

Chat::ChatClientThread::~ChatClientThread()
{
}

bool ProcessUserNameUpdate(Networking::Serialization::Deserializer& dr, Chat::UserManager* users)
{
	Chat::User* user;
	u64 userID;
	if (!dr.Read(userID))
	{
		return false;
	}
	user = users->GetOrCreateUser(userID);
	std::string tmp;
	u64 size;
	if (!dr.Read(size)) return false;
	tmp.reserve(size);
	if (!dr.Read(reinterpret_cast<u8*>(tmp.data()), size)) return false;
	user->userName = tmp;
	return true;
}

Chat::ActionData SendUserName(Chat::User* user)
{
	Chat::ActionData data;
	Networking::Serialization::Serializer sr;
	sr.Write(user->userID);
	sr.Write(user->userName.size());
	sr.Write(reinterpret_cast<u8*>(user->userName.data()), user->userName.size());
	data.type = Chat::Action::USER_UPDATE_NAME;
	data.dataSize = sr.GetBufferSize();
	u8* tmpbuffer = new u8[data.dataSize];
	std::copy(sr.GetBuffer(), sr.GetBuffer() + data.dataSize, tmpbuffer);
	data.data = tmpbuffer;
	return data;
}

bool ProcessUserColorUpdate(Networking::Serialization::Deserializer& dr, Chat::UserManager* users)
{
	Chat::User* user;
	u64 userID;
	if (!dr.Read(userID))
	{
		return false;
	}
	user = users->GetOrCreateUser(userID);
	if (!dr.Read(user->userColor.x) || !dr.Read(user->userColor.y) || !dr.Read(user->userColor.z)) return false;
	return true;
}

Chat::ActionData SendUserColor(Chat::User* user)
{
	Chat::ActionData data;
	Networking::Serialization::Serializer sr;
	sr.Write(user->userID);
	sr.Write(user->userColor.x);
	sr.Write(user->userColor.y);
	sr.Write(user->userColor.z);
	data.type = Chat::Action::USER_UPDATE_COLOR;
	data.dataSize = sr.GetBufferSize();
	u8* tmpbuffer = new u8[data.dataSize];
	std::copy(sr.GetBuffer(), sr.GetBuffer() + data.dataSize, tmpbuffer);
	data.data = tmpbuffer;
	return data;
}

bool ProcessUserIconUpdate(Networking::Serialization::Deserializer& dr, Chat::UserManager* users, Resources::TextureManager* textures)
{
	Chat::User* user;
	u64 userID;
	if (!dr.Read(userID))
	{
		return false;
	}
	user = users->GetOrCreateUser(userID);
	std::string texPath;
	u64 nameSize;
	if (!dr.Read(nameSize)) return false;
	texPath.reserve(nameSize);
	if (!dr.Read(reinterpret_cast<u8*>(texPath.data()), nameSize)) return false;
	std::string texExt;
	u64 extSize;
	if (!dr.Read(extSize)) return false;
	texExt.reserve(extSize);
	if (!dr.Read(reinterpret_cast<u8*>(texExt.data()), extSize)) return false;
	texPath = Maths::Util::GetHex(userID) + texPath;
	Resources::Texture* tex = textures->GetOrCreateTexture(texPath);
	Maths::IVec2 resolution;
	if (!dr.Read(resolution.x) || !dr.Read(resolution.y) || resolution.x < 16 || resolution.y < 16 || resolution.x > 256 || resolution.y > 16) return false;
	u64 texDataSize;
	if (!dr.Read(texDataSize) || texDataSize > 0x40000) return false;
	u8* data = new u8[texDataSize];
	if (!dr.Read(data, texDataSize))
	{
		delete[] data;
		return false;
	}
	if (tex->LoadFromMemory(texDataSize, data, texExt, texPath, resolution) != TextureError::NONE) return false;
	return true;
}

Chat::ActionData SendUserIcon(Chat::User* user)
{
	Chat::ActionData data;
	Networking::Serialization::Serializer sr;
	sr.Write(user->userID);
	sr.Write(user->userTex->GetPath().size());
	sr.Write(reinterpret_cast<const u8*>(user->userTex->GetPath().c_str()), user->userTex->GetPath().size());
	sr.Write(user->userTex->GetFileType().size());
	sr.Write(reinterpret_cast<const u8*>(user->userTex->GetFileType().c_str()), user->userTex->GetFileType().size());
	sr.Write(user->userTex->GetTextureWidth());
	sr.Write(user->userTex->GetTextureHeight());
	sr.Write(user->userTex->GetFileDataSize());
	sr.Write(user->userTex->GetFileData(), user->userTex->GetFileDataSize());
	data.type = Chat::Action::USER_UPDATE_ICON;
	data.dataSize = sr.GetBufferSize();
	u8* tmpbuffer = new u8[data.dataSize];
	std::copy(sr.GetBuffer(), sr.GetBuffer() + data.dataSize, tmpbuffer);
	data.data = tmpbuffer;
	return data;
}

bool ProcessTextMessage(Networking::Serialization::Deserializer& dr, Chat::UserManager* users, Chat::ChatManager* manager)
{
	Chat::User* user;
	u64 userID;
	u64 messID;
	u64 mTime;
	std::string tmp;
	u64 size;
	if (!dr.Read(mTime) || !dr.Read(userID) || !dr.Read(messID))
	{
		return false;
	}
	user = users->GetOrCreateUser(userID);
	if (!dr.Read(size)) return false;
	tmp.reserve(size);
	if (!dr.Read(reinterpret_cast<u8*>(tmp.data()), size)) return false;
	user->userName = tmp;
	std::unique_ptr<Chat::TextMessage> mess = std::make_unique<Chat::TextMessage>(tmp, users->GetUser(userID), mTime, messID);
	manager->ReceiveMessage(std::move(mess));
	return true;
}

void Chat::ChatClientThread::Update(f32 dt, ChatManager* manager, UserManager* users, Resources::TextureManager* textures)
{
	if (!signal.Load())
	{
		for (auto& a : actions)
		{
			std::unique_ptr<Chat::ConnectionMessage> mess;
			u64 userID;
			u64 messID;
			u64 mTime;
			Networking::Serialization::Deserializer dr = Networking::Serialization::Deserializer(static_cast<u8*>(a.data), a.dataSize);
			switch (a.type)
			{
			case Action::PING:
				break;
			case Action::USER_CONNECT:
				if (!dr.Read(mTime) || !dr.Read(userID) || !dr.Read(messID))
				{
					continue;
				}
				mess = std::make_unique<Chat::ConnectionMessage>(true, users->GetUser(userID), mTime, messID);
				manager->ReceiveMessage(std::move(mess));
				break;
			case Action::USER_DISCONNECT:
				if (!dr.Read(mTime) || !dr.Read(userID) || !dr.Read(messID))
				{
					continue;
				}
				mess = std::make_unique<Chat::ConnectionMessage>(false, users->GetUser(userID), mTime, messID);
				manager->ReceiveMessage(std::move(mess));
				break;
			case Action::MESSAGE_TEXT:
				ProcessTextMessage(dr, users, manager);
				break;
			case Action::MESSAGE_IMAGE:
				// TODO
				break;
			case Action::USER_UPDATE_NAME:
				ProcessUserNameUpdate(dr, users);
				break;
			case Action::USER_UPDATE_COLOR:
				ProcessUserColorUpdate(dr, users);
				break;
			case Action::USER_UPDATE_ICON:
				ProcessUserIconUpdate(dr, users, textures);
				break;
			default:
				std::cout << "Warning, Invalid action type" << std::endl;
				break;
			}
			delete[] a.data;
		}
		ChatNetworkThread::Update(dt);
	}
}

void Chat::ChatClientThread::Connect()
{
	if (state != ChatNetworkState::DISCONNECTED || connect.Load()) return;
	connect.Store(true);
}

void Chat::ChatClientThread::ThreadFunc()
{
	while (!shouldQuit.Load())
	{
		if (signal.Load())
		{
			std::vector<ActionData> response;
			if (state == ChatNetworkState::CONNECTED)
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
			}
			else if (connect.Load() && state != ChatNetworkState::WAITING_CONNECTION)
			{
				client.connect(address);

			}
			actions.clear();
			client.receive();
			if (state == ChatNetworkState::CONNECTED) client.processSend();
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
							response.push_back(SendUserName(self));
							response.push_back(SendUserColor(self));
							response.push_back(SendUserIcon(self));
						}
					}
				}
				else if (m->is<Networking::Messages::UserData>())
				{
					auto ud = m->as<Networking::Messages::UserData>();
					Networking::Serialization::Deserializer dr(ud->data.data(), ud->data.size());
					while (dr.CursorPos() < dr.BufferSize())
					{
						ActionData action;
						if (!dr.Read(reinterpret_cast<u8&>(action.type)) || !dr.Read(action.dataSize))
						{
							std::cout << "Warning, Corrupted message found!" << std::endl;
							break;
						}
						if (action.dataSize != 0)
						{
							u8* tmpData = new u8[action.dataSize];
							if (!dr.Read(tmpData, action.dataSize))
							{
								std::cout << "Warning, Corrupted message found!" << std::endl;
								break;
							}
							action.data = tmpData;
						}
						actions.push_back(std::move(action));
					}
					
				}
				else if (m->is<Networking::Messages::Disconnection>())
				{
					state = ChatNetworkState::DISCONNECTED;
				}
			}
			Networking::Serialization::Serializer sr;
			for (size_t i = 0; i < response.size(); i++)
			{
				sr.Write(static_cast<u8>(response[i].type));
				sr.Write(response[i].dataSize);
				if (response[i].dataSize != 0)
				{
					sr.Write(static_cast<u8*>(response[i].data), response[i].dataSize);
				}
			}
			client.sendTo(address, sr.GetBuffer(), sr.GetBufferSize(), 0);
			signal.Store(true);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	client.disconnect(address);
	client.processSend();
}

Chat::ChatServerThread::ChatServerThread(Networking::Address& addressIn, User* selfUser) : ChatNetworkThread(selfUser)
{
	address = addressIn;
	if (!client.init(address.port()))
	{
		std::cout << "Erreur d’initialisation du socket : " << Networking::Sockets::GetError();
		return;
	}
	state = ChatNetworkState::CONNECTED;
	t = std::thread(&ChatServerThread::ThreadFunc, this);
}

Chat::ChatServerThread::~ChatServerThread()
{
}

static u64 messageCounter = 0;
bool Chat::ChatServerThread::ProcessServerTextMessage(Networking::Serialization::Deserializer& dr, Chat::UserManager* users, Chat::ChatManager* manager)
{
	Chat::User* user;
	u64 userID;
	u64 messID = messageCounter;
	messageCounter++;
	u64 mTime;
	std::string tmp;
	u64 size;
	if (!dr.Read(mTime) || !dr.Read(userID))
	{
		return false;
	}
	user = users->GetOrCreateUser(userID);
	if (!dr.Read(size)) return false;
	tmp.reserve(size);
	if (!dr.Read(reinterpret_cast<u8*>(tmp.data()), size)) return false;
	user->userName = tmp;
	std::unique_ptr<Chat::TextMessage> mess = std::make_unique<Chat::TextMessage>(tmp, users->GetUser(userID), mTime, messID);
	manager->ReceiveMessage(std::move(mess));
	Networking::Serialization::Serializer sr;
	sr.Write(mTime);
	sr.Write(userID);
	sr.Write(messID);
	sr.Write(size);
	sr.Write(reinterpret_cast<u8*>(tmp.data()), size);
	Chat::ActionData action;
	action.type = Chat::Action::MESSAGE_TEXT;
	action.dataSize = sr.GetBufferSize();
	u8* tmpbuffer = new u8[action.dataSize];
	std::copy(sr.GetBuffer(), sr.GetBuffer() + action.dataSize, tmpbuffer);
	action.data = tmpbuffer;
	actionQueue.push_back(action);
	return true;
}

bool Chat::ChatServerThread::ProcessServerUserColorUpdate(Networking::Serialization::Deserializer& dr, Chat::UserManager* users)
{
	Chat::User* user;
	u64 userID;
	if (!dr.Read(userID))
	{
		return false;
	}
	user = users->GetOrCreateUser(userID);
	if (!dr.Read(user->userColor.x) || !dr.Read(user->userColor.y) || !dr.Read(user->userColor.z)) return false;

	Networking::Serialization::Serializer sr;
	sr.Write(user->userID);
	sr.Write(user->userColor.x);
	sr.Write(user->userColor.y);
	sr.Write(user->userColor.z);
	Chat::ActionData action;
	action.type = Chat::Action::USER_UPDATE_COLOR;
	action.dataSize = sr.GetBufferSize();
	u8* tmpbuffer = new u8[action.dataSize];
	std::copy(sr.GetBuffer(), sr.GetBuffer() + action.dataSize, tmpbuffer);
	action.data = tmpbuffer;
	actionQueue.push_back(action);
	return true;
}

bool Chat::ChatServerThread::ProcessServerUserNameUpdate(Networking::Serialization::Deserializer& dr, Chat::UserManager* users)
{
	u64 networkID;
	Chat::User* user;
	u64 userID;
	if (!dr.Read(networkID) || !dr.Read(userID))
	{
		return false;
	}
	user = users->GetOrCreateUser(userID);
	std::string tmp;
	u64 size;
	if (!dr.Read(size)) return false;
	tmp.reserve(size);
	if (!dr.Read(reinterpret_cast<u8*>(tmp.data()), size)) return false;
	user->userName = tmp;
	Networking::Serialization::Serializer sr;
	sr.Write(user->userID);
	sr.Write(user->userName.size());
	sr.Write(reinterpret_cast<u8*>(user->userName.data()), user->userName.size());
	auto last = acceptedClients.before_begin();
	for (auto t = acceptedClients.begin(); t != acceptedClients.end(); t++)
	{
		if (*t == networkID)
		{
			// TODO make connexion message
			acceptedClients.erase_after(last);
			break;
		}
		last = t;
	}
	Chat::ActionData action;
	action.type = Chat::Action::USER_UPDATE_NAME;
	action.dataSize = sr.GetBufferSize();
	u8* tmpbuffer = new u8[action.dataSize];
	std::copy(sr.GetBuffer(), sr.GetBuffer() + action.dataSize, tmpbuffer);
	action.data = tmpbuffer;
	actionQueue.push_back(action);
	
	return true;
}

void Chat::ChatServerThread::Update(f32 dt, ChatManager* manager, UserManager* users, Resources::TextureManager* textures)
{
	if (!signal.Load())
	{
		for (auto& a : actions)
		{
			Networking::Serialization::Deserializer dr = Networking::Serialization::Deserializer(static_cast<u8*>(a.data), a.dataSize);
			switch (a.type)
			{
			case Action::PING:
				break;
			case Action::USER_CONNECT:
				break;
			case Action::USER_DISCONNECT:
				break;
			case Action::MESSAGE_TEXT:
				ProcessServerTextMessage(dr, users, manager);
				break;
			case Action::MESSAGE_IMAGE:
				// TODO
				break;
			case Action::USER_UPDATE_NAME:
				ProcessServerUserNameUpdate(dr, users);
				break;
			case Action::USER_UPDATE_COLOR:
				ProcessServerUserColorUpdate(dr, users);
				break;
			case Action::USER_UPDATE_ICON:
				// TODO
				break;
			default:
				std::cout << "Warning, Invalid action type" << std::endl;
				break;
			}
			delete[] a.data;
		}
		ChatNetworkThread::Update(dt);
	}
}

void Chat::ChatServerThread::ThreadFunc()
{
	while (!shouldQuit.Load())
	{
		if (signal.Load())
		{
			std::vector<ActionData> response;
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
			client.broadCast(sr.GetBuffer(), sr.GetBufferSize(), 0);
			actions.clear();
			client.receive();
			client.processSend();
			auto v = client.poll();
			for (auto& m : v)
			{
				if (m->is<Networking::Messages::IncomingConnection>())
				{
					client.connect(m->as<Networking::Messages::IncomingConnection>()->emitter());
					Networking::Serialization::Serializer sr;
					sr.Write(time(nullptr));
					sr.Write(time(nullptr));
					sr.Write(time(nullptr));
					acceptedClients.push_front(m->emmiterId());
				}
				else if (m->is<Networking::Messages::Connection>())
				{
					// Should not happens on server side
				}
				else if (m->is<Networking::Messages::UserData>())
				{
					auto ud = m->as<Networking::Messages::UserData>();
					Networking::Serialization::Deserializer dr(ud->data.data(), ud->data.size());
					while (dr.CursorPos() < dr.BufferSize())
					{
						ActionData action;
						if (!dr.Read(reinterpret_cast<u8&>(action.type)) || !dr.Read(action.dataSize))
						{
							std::cout << "Warning, Corrupted message found!" << std::endl;
							break;
						}
						if (action.dataSize != 0)
						{
							if (action.type == Action::USER_UPDATE_NAME)
							{
								u8* tmpData = new u8[action.dataSize + 8];
								Networking::Serialization::Serializer tmpSR;
								tmpSR.Write(m->emmiterId());
								std::copy(tmpSR.GetBuffer(), tmpSR.GetBuffer() + 8, tmpData);
								if (!dr.Read(tmpData + 8, action.dataSize))
								{
									std::cout << "Warning, Corrupted message found!" << std::endl;
									break;
								}
								action.data = tmpData;
							}
							else
							{
								u8* tmpData = new u8[action.dataSize];
								if (!dr.Read(tmpData, action.dataSize))
								{
									std::cout << "Warning, Corrupted message found!" << std::endl;
									break;
								}
								action.data = tmpData;
							}
						}
						actions.push_back(std::move(action));
					}

				}
				else if (m->is<Networking::Messages::Disconnection>())
				{
					client.disconnect(m->as<Networking::Messages::Disconnection>()->emitter());
				}
			}
			signal.Store(true);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	client.disconnect(address);
	client.processSend();
}
