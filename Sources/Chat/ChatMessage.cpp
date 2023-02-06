#include "Chat/ChatMessage.hpp"

#include <time.h>

#include "Networking/Serialization/Serializer.hpp"

using namespace Chat;

float TextMessage::MaxWidth = 300.0f;

Chat::TextMessage::TextMessage(std::string& textIn, User* userIn, s64 tm, u64 id) : ChatMessage(userIn, tm, id)
{
	std::vector<std::string> text;
	height = ImGui::GetTextLineHeight() + 10;
	auto reset = [&](bool newLine)
	{
		text.push_back(std::string());
		if (newLine) message.push_back('\n');
		height += ImGui::GetTextLineHeight();
	};
	reset(false);
	for (auto& t : textIn)
	{
		if (t == '\n')
		{
			reset(true);
		}
		else if (t != '\0')
		{
			ImVec2 size = ImGui::CalcTextSize(text.back().c_str());
			if (size.y != ImGui::GetFontSize() || size.x > MaxWidth)
			{
				reset(true);
			}
			text.back().push_back(t);
			message.push_back(t);
		}
	}
	if (height < 60) height = 60;
}

void TextMessage::Draw() const
{
	ImVec2 pos = ImGui::GetCursorPos();
	DrawUser(pos);
	ImGui::Selectable(message.c_str());
	if (ImGui::BeginPopupContextItem(messageSTR))
	{
		if (ImGui::Button("Copy"))
		{
			ImGui::SetClipboardText(message.c_str());
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::IsAnyMouseDown() && !ImGui::IsItemHovered())
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::SetCursorPosX(pos.x);
}

Chat::ActionData Chat::TextMessage::Serialize() const
{
	Networking::Serialization::Serializer sr;
	sr.Write(unixTime);
	sr.Write(sender->userID);
	sr.Write(messageID);
	sr.Write(message.size());
	sr.Write(reinterpret_cast<const u8*>(message.data()), message.size());
	Chat::ActionData action;
	action.type = Chat::Action::MESSAGE_TEXT;
	action.data = std::vector(sr.GetBuffer(), sr.GetBuffer() + sr.GetBufferSize());
	return action;
}

Chat::ImageMessage::ImageMessage(Resources::Texture* img, User* userIn, s64 tm, u64 id) : ChatMessage(userIn, tm, id), tex(img)
{
	height = 210 + ImGui::GetTextLineHeightWithSpacing();
	width = tex->GetTextureWidth() * 200.0f / tex->GetTextureHeight();
}

Resources::Texture* Chat::ImageMessage::unloadedImg = nullptr;
void Chat::ImageMessage::Draw() const
{
	ImVec2 pos = ImGui::GetCursorPos();
	DrawUser(pos);
	if (tex->IsLoaded())
	{
		ImGui::Image((ImTextureID)tex->GetTextureID(), ImVec2(width, 200));
		if (ImGui::BeginPopupContextItem(messageSTR))
		{
			if (ImGui::Button("Save File"))
			{
				std::string path = std::string("Saved\\") + messageSTR;
				tex->SaveFileData(path);
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::IsAnyMouseDown() && !ImGui::IsItemHovered())
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
	else
	{
		ImGui::Image((ImTextureID)unloadedImg->GetTextureID(), ImVec2(200, 200));
		ImGui::SetCursorPos(ImVec2(pos.x, pos.y + 100));
		ImGui::ProgressBar(tex->GetLoadingCompletion(), ImVec2(200, 20));
	}
	ImGui::SetCursorPosX(pos.x);
}

Chat::ActionData Chat::ImageMessage::Serialize() const
{
	Networking::Serialization::Serializer sr;
	sr.Write(unixTime);
	sr.Write(sender->userID);
	sr.Write(messageID);
	sr.Write(tex->GetPath().size());
	sr.Write(reinterpret_cast<const u8*>(tex->GetPath().data()), tex->GetPath().size());
	tex->SerializeFile(sr);
	Chat::ActionData action;
	action.type = Chat::Action::MESSAGE_IMAGE;
	action.data = std::vector(sr.GetBuffer(), sr.GetBuffer() + sr.GetBufferSize());
	return action;
}

void Chat::ImageMessage::SetDefaultImage(Resources::Texture* imgIn)
{
	unloadedImg = imgIn;
}

Chat::ChatMessage::ChatMessage(User* s, s64 timeIn, u64 id) : sender(s), messageID(id)
{
	Maths::Util::GetHex(messageSTR, messageID);
	unixTime = timeIn;
	tm timeinfo;
	char buffer[80];

	if (!localtime_s(&timeinfo, &unixTime))
	{
		strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
		timestamp = buffer;
	}
}

void Chat::ChatMessage::DrawUser(ImVec2& pos) const
{
	ImGui::SetCursorPos(ImVec2(pos.x - 60, pos.y));
	ImGui::Image((ImTextureID)sender->userTex->GetTextureID(), ImVec2(50, 50));
	ImGui::SetCursorPos(ImVec2(pos.x, pos.y));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(sender->userColor.x, sender->userColor.y, sender->userColor.z, 1.0f));
	ImGui::TextUnformatted(sender->userName.c_str());
	ImGui::PopStyleColor();
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(127,127,127,255));
	ImGui::TextUnformatted(timestamp.c_str());
	ImGui::PopStyleColor();
	ImGui::SetCursorPosX(pos.x);
}

Chat::ConnectionMessage::ConnectionMessage(bool connected, User* userIn, s64 tm, u64 id) : ChatMessage(userIn, tm, id), connect(connected)
{
	height = ImGui::GetTextLineHeight() + 10;
}

void Chat::ConnectionMessage::Draw() const
{
	ImVec2 pos = ImGui::GetCursorPos();
	ImGui::SetCursorPos(ImVec2(pos.x, pos.y));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(sender->userColor.x, sender->userColor.y, sender->userColor.z, 1.0f));
	ImGui::TextUnformatted(sender->userName.c_str());
	ImGui::PopStyleColor();
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(127, 127, 127, 255));
	ImGui::TextUnformatted(connect ? "joined the chat" : "disconnected");
	ImGui::SameLine();
	ImGui::TextUnformatted(timestamp.c_str());
	ImGui::PopStyleColor();
	ImGui::SetCursorPosX(pos.x);
}

Chat::ActionData Chat::ConnectionMessage::Serialize() const
{
	Networking::Serialization::Serializer sr2;
	sr2.Write(unixTime);
	sr2.Write(sender->userID);
	sr2.Write(messageID);
	Chat::ActionData action;
	action.type = connect ? Chat::Action::USER_CONNECT : Chat::Action::USER_DISCONNECT;
	action.data = std::vector(sr2.GetBuffer(), sr2.GetBuffer() + sr2.GetBufferSize());
	return action;
}
