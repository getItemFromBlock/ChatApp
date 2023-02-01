#include "Chat/ChatManager.hpp"

#include <ImGUI/imgui.h>
#include <ImGUI-FileBrowser/imfilebrowser.h>
#include <ImGUI/imgui_stdlib.hpp>
#include <time.h>

using namespace Chat;

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
		ImGui::InputTextMultiline("##textInput", &currentText, ImVec2(0,0), ImGuiInputTextFlags_CtrlEnterForNewLine);
		bool valided = ImGui::IsItemDeactivatedAfterEdit();
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
		ImGui::End();
	}
	DrawPopup();
}

void Chat::ChatManager::ReceiveMessage(std::unique_ptr<ChatMessage>&& mess)
{
	totalHeight += mess->GetHeight() + 10;
	lastHeight = 0;
	setDown = true;
	messages.push_back(std::move(mess));
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
	ntwThread = std::make_unique<ChatNetworkThread>(users->GetUser(selfID));
}

void Chat::ClientChatManager::SendChatMessage()
{
	Networking::Serialization::Serializer sr;
	sr.Write(selfID);
	sr.Write(currentText.size());
	sr.Write(reinterpret_cast<u8*>(currentText.data()), currentText.size());
	u8* data = new u8[sr.GetBufferSize()];
	std::copy(sr.GetBuffer(), sr.GetBuffer() + sr.GetBufferSize(), data);
	ntwThread->PushAction(Action::MESSAGE_TEXT, data, sr.GetBufferSize());
	currentText.clear();
}

void Chat::ClientChatManager::Render()
{
	if (ntwThread->GetState() == ChatNetworkState::CONNECTED)
	{
		ChatManager::Render();
	}
	else if (ntwThread->GetState() == ChatNetworkState::DISCONNECTED)
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
					ad = Networking::Address::Any(isIPV6 ? Networking::Address::Type::IPv6 : Networking::Address::Type::IPv4, serverPort);
				}
				else
				{
					ad = Networking::Address::Address(serverAddress, serverPort);
				}
				if (ad.isValid())
				{
					ntwThread->SetAddress(ad);
					ntwThread->TryConnect();
				}
			}
			ImGui::End();
		}
	}
	else
	{
		if (ImGui::Begin("Connexion", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Text("Connecting to %s...", serverAddress.c_str());
			ImGui::End();
		}
	}
}
