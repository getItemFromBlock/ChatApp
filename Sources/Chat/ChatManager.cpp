#include "Chat/ChatManager.hpp"

#include <ImGUI/imgui.h>
#include <ImGUI-FileBrowser/imfilebrowser.h>
#include <ImGUI/imgui_stdlib.hpp>
#include <time.h>

using namespace Chat;

void Chat::ClientChatManager::Update()
{
	reinterpret_cast<ChatClientThread*>(ntwThread.get())->Update();
}

void Chat::ClientChatManager::RenderConnectionScreen()
{
	if (ImGui::Begin("Connexion", nullptr, ImGuiWindowFlags_NoCollapse))
	{
		ImGui::InputText("Server IP Address", &serverAddress);
		s32 tmpPort = serverPort;
		ImGui::InputInt("Server Port", &tmpPort);
		serverPort = Maths::Util::iclamp(tmpPort, 0, 0xffff);
		ImGui::Checkbox("Is IPV6", &isIPV6);
		if (ImGui::Button("Connect"))
		{
			Networking::Address ad;
			if (serverAddress.empty())
			{
				serverAddress = "127.0.0.1";
				isIPV6 = false;
			}
			ad = Networking::Address::Address(serverAddress, serverPort);
			if (ad.isValid())
			{
				ntwThread->SetAddress(ad);
				ntwThread->TryConnect();
			}
			else
			{
				ImGui::OpenPopup("Address Error");
			}
		}
		if (ImGui::BeginPopupModal("Address Error", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
			ImGui::TextUnformatted("Invalid address");
			if (ImGui::Button("Close"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor();
			ImGui::EndPopup();
		}
		ImGui::End();
	}
}

void Chat::ServerChatManager::Update()
{
	reinterpret_cast<ChatServerThread*>(ntwThread.get())->Update();
}

void Chat::ChatManager::UpdateUserName()
{
	ntwThread->PushAction(ntwThread->SendUserName(users->GetUser(selfID)));
}

void Chat::ChatManager::UpdateUserColor()
{
	ntwThread->PushAction(ntwThread->SendUserColor(users->GetUser(selfID)));
}

void Chat::ChatManager::UpdateUserIcon()
{
	ntwThread->PushAction(ntwThread->SendUserIcon(users->GetUser(selfID)));
}

void ChatManager::Render()
{
	if (setDown && lastHeight != 0)
	{
		setDown = false;
		ImGui::SetNextWindowScroll(ImVec2(0, lastHeight + messages.back()->GetHeight() + 10));
	}
	if (ImGui::Begin("Channel", nullptr, ImGuiWindowFlags_NoCollapse))
	{
		Chat::TextMessage::MaxWidth = ImGui::GetContentRegionMax().x - ImGui::GetWindowContentRegionMin().x - 90;
		ImGui::SetCursorPosX(70);
		float pos = totalHeight + 10;
		size_t index = messages.size();
		while (index)
		{
			pos -= messages[index-1]->GetHeight();
			ImGui::SetCursorPosY(pos);
			messages[index-1]->Draw();
			pos -= 10;
			ImGui::SetCursorPosY(pos);
			index--;
		}
		ImGui::End();
		ImGui::Begin("Channel", nullptr, ImGuiWindowFlags_NoCollapse);
		lastHeight = ImGui::GetScrollMaxY();
		ImGui::End();
	}
	if (ImGui::Begin("Send Message", nullptr, ImGuiWindowFlags_NoCollapse))
	{
		bool valided = ImGui::InputTextMultiline("##textInput", &currentText, ImVec2(0,0), ImGuiInputTextFlags_CtrlEnterForNewLine | ImGuiInputTextFlags_EnterReturnsTrue);
		if (currentText.size() > 32768) currentText.resize(32768);
		if ((ImGui::Button("Send Message") || valided) && !currentText.empty())
		{
			SendChatMessage();
		}
		ImGui::SameLine();
		if (ImGui::Button("Send Image"))
		{
			// TODO
			//browser->Open();
		}
		browser->Display();
		if (browser->HasSelected())
		{
			std::filesystem::path texPath = browser->GetSelected();
			std::string path = texPath.string();
			Resources::Texture* result = textures->GetOrCreateTexture(path);
			if (result->IsLoaded())
			{
				std::unique_ptr<ImageMessage> mes = std::make_unique<ImageMessage>(result, users->GetUser(selfID), time(nullptr));
				totalHeight += mes->GetHeight() + 10;
				messages.push_back(std::move(mes));
			}
			else
			{
				lastError = Resources::Texture::TryLoad(path.c_str(), result, Maths::Vec2(), Maths::Vec2(), 0x800000);
				if (lastError == TextureError::NONE)
				{
					std::unique_ptr<ImageMessage> mes = std::make_unique<ImageMessage>(result, users->GetUser(selfID), time(nullptr));
					totalHeight += mes->GetHeight() + 10;
					messages.push_back(std::move(mes));
				}
				else
				{
					ImGui::OpenPopup("Chat Texture Error");
				}
			}
			browser->ClearSelected();
		}
		DrawPopup();
		ImGui::End();
	}
}

void Chat::ChatManager::ReceiveMessage(std::unique_ptr<ChatMessage>&& mess)
{
	totalHeight += mess->GetHeight() + 10;
	lastHeight = 0;
	setDown = true;
	messages.push_back(std::move(mess));
}

void Chat::ChatManager::RetreiveMessages(std::vector<ActionData>& outputVec, u64 first, u64 last)
{
	for (u64 index = first; index < messages.size() && index < last; index++)
	{

	}
}

const std::vector<std::unique_ptr<ChatMessage>>& ChatManager::GetAllMessages()
{
	return messages;
}

void Chat::ChatManager::DrawPopup()
{
	if (ImGui::BeginPopupModal("Chat Texture Error", nullptr, ImGuiWindowFlags_NoCollapse))
	{
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
		ImGui::TextUnformatted("Error reading file:");
		switch (lastError)
		{
		case TextureError::NONE:
			ImGui::CloseCurrentPopup();
			break;
		case TextureError::NO_FILE:
			ImGui::TextUnformatted("Image Not Found!");
			break;
		case TextureError::FILE_TOO_BIG:
			ImGui::TextUnformatted("Image file is too big!\nMaximum is 8388608 (8M) bytes");
			break;
		case TextureError::IMG_TOO_SMALL:
			ImGui::TextUnformatted("Image resolution is too small!\nMinimum is / pixels");
			break;
		case TextureError::IMG_TOO_BIG:
			ImGui::TextUnformatted("Image resolution is too big!\nMaximum is / pixels");
			break;
		case TextureError::IMG_INVALID:
			ImGui::TextUnformatted("Image file is invalid or corrupted!");
			ImGui::TextUnformatted(Resources::Texture::GetSTBIError());
			break;
		case TextureError::OTHER:
			ImGui::TextUnformatted("Could not read Image file!");
			break;
		default:
			ImGui::CloseCurrentPopup();
			break;
		}
		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleColor();
		ImGui::EndPopup();
	}
}

Chat::ClientChatManager::ClientChatManager(UserManager* users, Resources::TextureManager* textures, u64 s, ImGui::FileBrowser* br) : ChatManager(users, textures, s, br)
{
	ntwThread = std::make_unique<ChatClientThread>(users->GetUser(selfID), this, users, textures);
}

void Chat::ClientChatManager::SendChatMessage()
{
	Networking::Serialization::Serializer sr;
	sr.Write(selfID);
	sr.Write(currentText.size());
	sr.Write(reinterpret_cast<u8*>(currentText.data()), currentText.size());
	ntwThread->PushAction(Action::MESSAGE_TEXT, sr.GetBuffer(), sr.GetBufferSize());
	currentText.clear();
}

void Chat::ClientChatManager::Render()
{
	switch (ntwThread->GetState())
	{
	case ChatNetworkState::DISCONNECTED:
		RenderConnectionScreen();
		break;
	case ChatNetworkState::CONNECTED:
		ChatManager::Render();
		break;
	case ChatNetworkState::WAITING_CONNECTION:
		if (ImGui::Begin("Join Chat", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Text("Connecting to %s...", serverAddress.c_str());
			ImGui::End();
		}
		break;
	case ChatNetworkState::CONNECTION_LOST:
		if (ImGui::Begin("Disconnected", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::TextUnformatted("Connection error :");
			ImGui::TextUnformatted(ntwThread->GetLastError());
			if (ImGui::Button("Back"))
			{
				ntwThread->ResetState();
			}
			ImGui::End();
		}
		break;
	default:
		break;
	}
}

Chat::ServerChatManager::ServerChatManager(UserManager* users, Resources::TextureManager* textures, u64 s, ImGui::FileBrowser* br) : ChatManager(users, textures, s, br)
{
	ntwThread = std::make_unique<ChatServerThread>(users->GetUser(selfID), this, users, textures);
}

void Chat::ServerChatManager::SendChatMessage()
{
	u64 messID = reinterpret_cast<ChatServerThread*>(ntwThread.get())->GetMessageCounter();
	u64 receivedTime = time(nullptr);
	std::unique_ptr<Chat::TextMessage> mess = std::make_unique<Chat::TextMessage>(currentText, users->GetUser(selfID), receivedTime, messID);
	ReceiveMessage(std::move(mess));
	Networking::Serialization::Serializer sr;
	sr.Write(receivedTime);
	sr.Write(selfID);
	sr.Write(messID);
	sr.Write(currentText.size());
	sr.Write(reinterpret_cast<u8*>(currentText.data()), currentText.size());
	ntwThread->PushAction(Chat::Action::MESSAGE_TEXT, sr.GetBuffer(), sr.GetBufferSize());
	currentText.clear();
}

void Chat::ServerChatManager::Render()
{
	if (ntwThread->GetState() == ChatNetworkState::CONNECTED)
	{
		ChatManager::Render();
	}
	else
	{
		if (ImGui::Begin("Connexion", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			s32 tmpPort = serverPort;
			ImGui::InputInt("Server Port", &tmpPort);
			serverPort = Maths::Util::iclamp(tmpPort, 0, 0xffff);
			ImGui::Checkbox("Is IPV6", &isIPV6);
			if (ImGui::Button("Create Chat"))
			{
				Networking::Address ad;
				ad = Networking::Address::Loopback(isIPV6 ? Networking::Address::Type::IPv6 : Networking::Address::Type::IPv4, serverPort);
				if (ad.isValid())
				{
					ntwThread->SetAddress(ad);
					ntwThread->TryConnect();
				}
			}
			ImGui::End();
		}
	}
}
