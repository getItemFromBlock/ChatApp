#include "Chat/ChatNetworkThread.hpp"

#include <iostream>

#include "Networking/UDP/Protocols/ReliableOrdered.hpp"
#include "Networking/Errors.hpp"
#include "Networking/Messages.hpp"
#include "Chat/ChatManager.hpp"

Chat::ChatNetworkThread::ChatNetworkThread(User* selfUser, ChatManager* managerIn, UserManager* usersIn, Resources::TextureManager* texturesIn) :
	self(selfUser), manager(managerIn), users(usersIn), textures(texturesIn)
{
	client.registerChannel<Networking::UDP::Protocols::ReliableOrdered>();
}

Chat::ChatNetworkThread::~ChatNetworkThread()
{
	shouldQuit.Store(true);
	t.join();
}

void Chat::ChatNetworkThread::Update()
{
	actions.clear();
	for (size_t i = 0; i < actionQueue.size(); i++)
	{
		actions.push_back(std::move(actionQueue[i]));
	}
	if (actions.size() == 0)
	{
		actions.push_back(std::move(ActionData(Action::PING, nullptr, 0)));
	}
	actionQueue.clear();
	signal.Store(true);
}

void Chat::ChatNetworkThread::PushAction(Action type, const u8* data, u64 dataSize)
{
	actionQueue.push_back(std::move(ActionData(type, data, dataSize)));
}

void Chat::ChatNetworkThread::PushAction(ActionData&& action)
{
	actionQueue.push_back(std::move(action));
}

Chat::ChatClientThread::ChatClientThread(User* selfUser, ChatManager* managerIn, UserManager* usersIn, Resources::TextureManager* texturesIn) :
	ChatNetworkThread(selfUser, managerIn, usersIn, texturesIn)
{
	if (!client.init(0))
	{
		std::cout << "Erreur d’initialisation du socket : " << Networking::Sockets::GetError();
		return;
	}
	t = std::thread(&ChatClientThread::ThreadFunc, this);
}

void Chat::ChatNetworkThread::SetAddress(Networking::Address& addressIn)
{
	address = addressIn;
}

Chat::ChatClientThread::~ChatClientThread()
{
}

bool Chat::ChatClientThread::ProcessUserNameUpdate(Networking::Serialization::Deserializer& dr, Chat::UserManager* users)
{
	Chat::User* user;
	u64 userID;
	if (!dr.Read(userID) || userID == self->userID)
	{
		return false;
	}
	user = users->GetOrCreateUser(userID);
	std::string tmp;
	u64 size;
	if (!dr.Read(size)) return false;
	tmp.resize(size);
	if (!dr.Read(reinterpret_cast<u8*>(tmp.data()), size)) return false;
	user->userName = tmp;
	return true;
}

Chat::ActionData Chat::ChatNetworkThread::SendUserName(Chat::User* user)
{
	Chat::ActionData data;
	Networking::Serialization::Serializer sr;
	sr.Write(user->userID);
	sr.Write(user->userName.size());
	sr.Write(reinterpret_cast<u8*>(user->userName.data()), user->userName.size());
	data.type = Chat::Action::USER_UPDATE_NAME;
	data.data = std::vector(sr.GetBuffer(), sr.GetBuffer() + sr.GetBufferSize());
	return data;
}

bool Chat::ChatClientThread::ProcessUserColorUpdate(Networking::Serialization::Deserializer& dr, Chat::UserManager* users)
{
	Chat::User* user;
	u64 userID;
	if (!dr.Read(userID) || userID == self->userID)
	{
		return false;
	}
	user = users->GetOrCreateUser(userID);
	if (!dr.Read(user->userColor.x) || !dr.Read(user->userColor.y) || !dr.Read(user->userColor.z)) return false;
	return true;
}

Chat::ActionData Chat::ChatNetworkThread::SendUserColor(Chat::User* user)
{
	Chat::ActionData data;
	Networking::Serialization::Serializer sr;
	sr.Write(user->userID);
	sr.Write(user->userColor.x);
	sr.Write(user->userColor.y);
	sr.Write(user->userColor.z);
	data.type = Chat::Action::USER_UPDATE_COLOR;
	data.data = std::vector(sr.GetBuffer(), sr.GetBuffer() + sr.GetBufferSize());
	return data;
}

bool Chat::ChatClientThread::ProcessUserIconUpdate(Networking::Serialization::Deserializer& dr, Chat::UserManager* users, Resources::TextureManager* textures)
{
	Chat::User* user;
	u64 userID;
	if (!dr.Read(userID) || userID == self->userID)
	{
		return false;
	}
	user = users->GetOrCreateUser(userID);
	std::string texPath;
	u64 nameSize;
	if (!dr.Read(nameSize)) return false;
	texPath.resize(nameSize);
	if (!dr.Read(reinterpret_cast<u8*>(texPath.data()), nameSize)) return false;
	std::string texExt;
	u64 extSize;
	if (!dr.Read(extSize)) return false;
	texExt.resize(extSize);
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

Chat::ActionData Chat::ChatNetworkThread::SendUserIcon(Chat::User* user)
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
	data.data = std::vector(sr.GetBuffer(), sr.GetBuffer() + sr.GetBufferSize());
	return data;
}

bool ProcessTextMessage(Networking::Serialization::Deserializer& dr, Chat::UserManager* users, Chat::ChatManager* manager)
{
	u64 userID;
	u64 messID;
	u64 mTime;
	std::string tmp;
	u64 size;
	if (!dr.Read(mTime) || !dr.Read(userID) || !dr.Read(messID))
	{
		return false;
	}
	if (!dr.Read(size)) return false;
	tmp.resize(size);
	if (!dr.Read(reinterpret_cast<u8*>(tmp.data()), size)) return false;
	std::unique_ptr<Chat::TextMessage> mess = std::make_unique<Chat::TextMessage>(tmp, users->GetOrCreateUser(userID), mTime, messID);
	manager->ReceiveMessage(std::move(mess));
	return true;
}

void Chat::ChatClientThread::Update()
{
	if (!signal.Load())
	{
		for (auto& a : actions)
		{
			std::unique_ptr<Chat::ConnectionMessage> mess;
			u64 userID;
			u64 messID;
			u64 mTime;
			Networking::Serialization::Deserializer dr = Networking::Serialization::Deserializer(a.data.data(), a.data.size());
			switch (a.type)
			{
			case Action::PING:
				break;
			case Action::USER_CONNECT:
				if (!dr.Read(mTime) || !dr.Read(userID) || !dr.Read(messID))
				{
					continue;
				}
				mess = std::make_unique<Chat::ConnectionMessage>(true, users->GetOrCreateUser(userID), mTime, messID);
				manager->ReceiveMessage(std::move(mess));
				break;
			case Action::USER_DISCONNECT:
				if (!dr.Read(mTime) || !dr.Read(userID) || !dr.Read(messID))
				{
					continue;
				}
				mess = std::make_unique<Chat::ConnectionMessage>(false, users->GetOrCreateUser(userID), mTime, messID);
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
		}
		ChatNetworkThread::Update();
	}
}

void Chat::ChatClientThread::TryConnect()
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
					sr.Write(actions[i].data.size());
					if (actions[i].data.size() != 0)
					{
						sr.Write(actions[i].data.data(), actions[i].data.size());
					}
				}
				if (sr.GetBufferSize() > 0)
				{
					client.sendTo(address, sr.GetBuffer(), sr.GetBufferSize(), 0);
				}
			}
			else if (connect.Load() && state == ChatNetworkState::DISCONNECTED)
			{
				client.connect(address);
				connect.Store(false);
				state = ChatNetworkState::WAITING_CONNECTION;
			}
			actions.clear();
			client.receive();
			if (state == ChatNetworkState::CONNECTED || state == ChatNetworkState::WAITING_CONNECTION) client.processSend();
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
						switch (ud->result)
						{
						case Networking::Messages::Connection::Result::Success:
							state = ChatNetworkState::CONNECTED;
							response.push_back(SendUserName(self));
							response.push_back(SendUserColor(self));
							response.push_back(SendUserIcon(self));
							break;
						case Networking::Messages::Connection::Result::Failed:
							lastError = "Could not connect to server";
							state = ChatNetworkState::CONNECTION_LOST;
							break;
						case Networking::Messages::Connection::Result::Refused:
							lastError = "Connection refused";
							state = ChatNetworkState::CONNECTION_LOST;
							break;
						case Networking::Messages::Connection::Result::TimedOut:
							lastError = "Connection timed out";
							state = ChatNetworkState::CONNECTION_LOST;
							break;
						default:
							lastError = "Unknown error";
							state = ChatNetworkState::CONNECTION_LOST;
							break;
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
						u64 tmpSize;
						if (!dr.Read(reinterpret_cast<u8&>(action.type)) || !dr.Read(tmpSize))
						{
							std::cout << "Warning, Corrupted message found!" << std::endl;
							break;
						}
						if (tmpSize != 0)
						{
							action.data.resize(tmpSize);
							if (!dr.Read(action.data.data(), action.data.size()))
							{
								std::cout << "Warning, Corrupted message found!" << std::endl;
								break;
							}
						}
						actions.push_back(std::move(action));
					}
					
				}
				else if (m->is<Networking::Messages::Disconnection>())
				{
					if (m->as<Networking::Messages::Disconnection>()->reason == Networking::Messages::Disconnection::Reason::Disconnected)
					{
						lastError = "Disconnected from server";
					}
					else
					{
						lastError = "Connection lost with server";
					}
					state = ChatNetworkState::CONNECTION_LOST;
				}
			}
			Networking::Serialization::Serializer sr;
			for (size_t i = 0; i < response.size(); i++)
			{
				sr.Write(static_cast<u8>(response[i].type));
				sr.Write(response[i].data.size());
				if (response[i].data.size() != 0)
				{
					sr.Write(response[i].data.data(), response[i].data.size());
				}
			}
			if(sr.GetBufferSize() > 0) client.sendTo(address, sr.GetBuffer(), sr.GetBufferSize(), 0);
			signal.Store(false);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
	if (address.isValid())
	{
		client.disconnect(address);
		client.processSend();
		client.release();
	}
}

Chat::ChatServerThread::ChatServerThread(User* selfUser, ChatManager* managerIn, UserManager* usersIn, Resources::TextureManager* texturesIn) :
	ChatNetworkThread(selfUser, managerIn, usersIn, texturesIn)
{
	t = std::thread(&ChatServerThread::ThreadFunc, this);
}

Chat::ChatServerThread::~ChatServerThread()
{
}

static u64 messageCounter = 0;
u64 Chat::ChatServerThread::GetMessageCounter()
{
	return messageCounter++;
}

void Chat::ChatServerThread::TryConnect()
{
	if (!client.init(address.port()))
	{
		std::cout << "Erreur d’initialisation du socket : " << Networking::Sockets::GetError();
		return;
	}
	else
	{
		state = ChatNetworkState::CONNECTED;
	}
}

bool Chat::ChatServerThread::ProcessServerTextMessage(Networking::Serialization::Deserializer& dr, Chat::UserManager* users, Chat::ChatManager* manager)
{
	u64 userID;
	u64 messID = GetMessageCounter();
	std::string tmp;
	u64 size;
	if (!dr.Read(userID))
	{
		return false;
	}
	if (!dr.Read(size)) return false;
	tmp.resize(size);
	if (!dr.Read(reinterpret_cast<u8*>(tmp.data()), size)) return false;
	u64 receivedTime = time(nullptr);
	std::unique_ptr<Chat::TextMessage> mess = std::make_unique<Chat::TextMessage>(tmp, users->GetOrCreateUser(userID), receivedTime, messID);
	actionQueue.push_back(std::move(mess->Serialize()));
	manager->ReceiveMessage(std::move(mess));
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
	action.data = std::vector(sr.GetBuffer(), sr.GetBuffer() + sr.GetBufferSize());
	actionQueue.push_back(std::move(action));
	return true;
}

bool Chat::ChatServerThread::ProcessServerUserNameUpdate(Networking::Serialization::Deserializer& dr, Chat::UserManager* users, Chat::ChatManager* manager)
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
	tmp.resize(size);
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
			user->networkID = networkID;
			u64 messID = GetMessageCounter();
			u64 receivedTime = time(nullptr);
			std::unique_ptr<Chat::ConnectionMessage> mess = std::make_unique<Chat::ConnectionMessage>(true, users->GetOrCreateUser(userID), receivedTime, messID);
			manager->ReceiveMessage(std::move(mess));

			Networking::Serialization::Serializer sr2;
			sr2.Write(receivedTime);
			sr2.Write(user->userID);
			sr2.Write(messID);
			Chat::ActionData action;
			action.type = Chat::Action::USER_CONNECT;
			action.data = std::vector(sr2.GetBuffer(), sr2.GetBuffer() + sr2.GetBufferSize());
			actionQueue.push_back(std::move(action));

			acceptedClients.erase_after(last);
			break;
		}
		last = t;
	}
	Chat::ActionData action;
	action.type = Chat::Action::USER_UPDATE_NAME;
	action.data = std::vector(sr.GetBuffer(), sr.GetBuffer() + sr.GetBufferSize());
	actionQueue.push_back(std::move(action));
	
	return true;
}

bool Chat::ChatServerThread::ProcessServerUserIconUpdate(Networking::Serialization::Deserializer& dr, Chat::UserManager* users, Resources::TextureManager* textures)
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
	texPath.resize(nameSize);
	if (!dr.Read(reinterpret_cast<u8*>(texPath.data()), nameSize)) return false;
	std::string texExt;
	u64 extSize;
	if (!dr.Read(extSize)) return false;
	texExt.resize(extSize);
	if (extSize != 0 && !dr.Read(reinterpret_cast<u8*>(texExt.data()), extSize)) return false;
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
	Chat::ActionData action;
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
	action.type = Chat::Action::USER_UPDATE_ICON;
	action.data = std::vector(sr.GetBuffer(), sr.GetBuffer() + sr.GetBufferSize());
	actionQueue.push_back(std::move(action));
	return true;
}

bool Chat::ChatServerThread::ProcessServerUserDisconnection(Networking::Serialization::Deserializer& dr, Chat::UserManager* users, Chat::ChatManager* manager)
{
	u64 netID;
	if (!dr.Read(netID)) return false;
	User* user = users->GetUserWithNetID(netID);
	if (!user) return false;
	u64 messID = GetMessageCounter();
	u64 receivedTime = time(nullptr);
	std::unique_ptr<Chat::ConnectionMessage> mess = std::make_unique<Chat::ConnectionMessage>(false, user, receivedTime, messID);
	actionQueue.push_back(std::move(mess->Serialize()));
	manager->ReceiveMessage(std::move(mess));
	return true;
}

bool Chat::ChatServerThread::ProcessServerUserConnection(const Networking::Address& clientIn, Chat::UserManager* users, Chat::ChatManager* manager)
{
	std::vector<ActionData> tmpActions;
	Networking::Serialization::Serializer sr;
	for (auto& u : users->GetAllUsers())
	{
		if (u.first == 0) continue; // no need to send the default users' data
		tmpActions.push_back(SendUserName(u.second.get()));
		tmpActions.push_back(SendUserColor(u.second.get()));
		tmpActions.push_back(SendUserIcon(u.second.get()));
	}
	for (auto& m : manager->GetAllMessages())
	{
		tmpActions.push_back(m->Serialize());
	}
	for (size_t i = 0; i < tmpActions.size(); i++)
	{
		sr.Write(static_cast<u8>(tmpActions[i].type));
		sr.Write(tmpActions[i].data.size());
		if (tmpActions[i].data.size() != 0)
		{
			sr.Write(tmpActions[i].data.data(), tmpActions[i].data.size());
		}
	}
	if (sr.GetBufferSize() > 0)
	{
		client.sendTo(clientIn, sr.GetBuffer(), sr.GetBufferSize(), 0);
	}
	return true;
}

void Chat::ChatServerThread::Update()
{
	if (!signal.Load())
	{
		for (auto& a : actions)
		{
			Networking::Serialization::Deserializer dr = Networking::Serialization::Deserializer(a.data.data(), a.data.size());
			switch (a.type)
			{
			case Action::PING:
				break;
			case Action::USER_CONNECT:
				break;
			case Action::USER_DISCONNECT:
				ProcessServerUserDisconnection(dr, users, manager);
				break;
			case Action::MESSAGE_TEXT:
				ProcessServerTextMessage(dr, users, manager);
				break;
			case Action::MESSAGE_IMAGE:
				// TODO
				break;
			case Action::USER_UPDATE_NAME:
				ProcessServerUserNameUpdate(dr, users, manager);
				break;
			case Action::USER_UPDATE_COLOR:
				ProcessServerUserColorUpdate(dr, users);
				break;
			case Action::USER_UPDATE_ICON:
				ProcessServerUserIconUpdate(dr, users, textures);
				break;
			default:
				std::cout << "Warning, Invalid action type" << std::endl;
				break;
			}
		}
		ChatNetworkThread::Update();
	}
}

void Chat::ChatServerThread::ThreadFunc()
{
	while (!shouldQuit.Load())
	{
		if (state == ChatNetworkState::CONNECTED && signal.Load())
		{
			std::vector<ActionData> response;
			Networking::Serialization::Serializer sr;
			for (size_t i = 0; i < actions.size(); i++)
			{
				sr.Write(static_cast<u8>(actions[i].type));
				sr.Write(actions[i].data.size());
				if (actions[i].data.size() != 0)
				{
					sr.Write(actions[i].data.data(), actions[i].data.size());
				}
			}
			if (sr.GetBufferSize() > 0)
			{
				client.broadCast(sr.GetBuffer(), sr.GetBufferSize(), 0);
			}
			actions.clear();
			client.receive();
			client.processSend();
			auto v = client.poll();
			for (auto& m : v)
			{
				if (m->is<Networking::Messages::IncomingConnection>())
				{
					client.connect(m->emitter());
					acceptedClients.push_front(m->emitterId());
					ProcessServerUserConnection(m->emitter(), users, manager);
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
						u64 tmpSize;
						if (!dr.Read(reinterpret_cast<u8&>(action.type)) || !dr.Read(tmpSize))
						{
							std::cout << "Warning, Corrupted message found!" << std::endl;
							break;
						}
						if (tmpSize != 0)
						{
							if (action.type == Action::USER_UPDATE_NAME)
							{
								action.data.resize(tmpSize + 8);
								Networking::Serialization::Serializer tmpSR;
								tmpSR.Write(m->emitterId());
								std::copy(tmpSR.GetBuffer(), tmpSR.GetBuffer() + 8, action.data.data());
								if (!dr.Read(action.data.data() + 8, tmpSize))
								{
									std::cout << "Warning, Corrupted message found!" << std::endl;
									break;
								}
							}
							else
							{
								action.data.resize(tmpSize);
								if (!dr.Read(action.data.data(), tmpSize))
								{
									std::cout << "Warning, Corrupted message found!" << std::endl;
									break;
								}
							}
						}
						actions.push_back(std::move(action));
					}
				}
				else if (m->is<Networking::Messages::Disconnection>())
				{
					client.disconnect(m->as<Networking::Messages::Disconnection>()->emitter());
					ActionData action;
					action.type = Action::USER_DISCONNECT;
					Networking::Serialization::Serializer sr;
					sr.Write(m->emitterId());
					action.data.resize(sr.GetBufferSize());
					std::copy(sr.GetBuffer(), sr.GetBuffer() + sr.GetBufferSize(), action.data.data());
					actions.push_back(std::move(action));
				}
			}
			Networking::Serialization::Serializer sr2;
			for (size_t i = 0; i < response.size(); i++)
			{
				sr2.Write(static_cast<u8>(response[i].type));
				sr2.Write(response[i].data.size());
				if (response[i].data.size() != 0)
				{
					sr2.Write(response[i].data.data(), response[i].data.size());
				}
			}
			if (sr2.GetBufferSize() > 0) client.broadCast(sr2.GetBuffer(), sr2.GetBufferSize(), 0);
			signal.Store(false);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
	if (address.isValid())
	{
		client.disconnectAll();
		client.processSend();
		client.release();
	}
}
