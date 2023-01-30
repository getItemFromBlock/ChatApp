#include "Chat/ChatMessage.hpp"

#include <time.h>

using namespace Chat;

float TextMessage::MaxWidth = 300.0f;

Chat::TextMessage::TextMessage(std::string& textIn, User* userIn, u64 tm, u64 id) : ChatMessage(userIn, tm, id)
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

void TextMessage::Draw()
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

Chat::ImageMessage::ImageMessage(Resources::Texture* img, User* userIn, u64 tm, u64 id) : ChatMessage(userIn, tm, id), tex(img)
{
	height = 210 + ImGui::GetTextLineHeightWithSpacing();
	width = tex->GetTextureWidth() * 200.0f / tex->GetTextureHeight();
}

void Chat::ImageMessage::Draw()
{
	ImVec2 pos = ImGui::GetCursorPos();
	DrawUser(pos);
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
	ImGui::SetCursorPosX(pos.x);
}

Chat::ChatMessage::ChatMessage(User* s, u64 timeIn, u64 id) : sender(s), messageID(id)
{
	Maths::Util::GetHex(messageSTR, messageID);
	time_t rawtime = timeIn;
	tm timeinfo;
	char buffer[80];

	time(&rawtime);
	if (!localtime_s(&timeinfo, &rawtime))
	{
		strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
		timestamp = buffer;
	}
}

void Chat::ChatMessage::DrawUser(ImVec2& pos)
{
	ImGui::SetCursorPos(ImVec2(pos.x - 60, pos.y));
	ImGui::Image((ImTextureID)sender->userTex->GetTextureID(), ImVec2(50, 50));
	ImGui::SetCursorPos(ImVec2(pos.x, pos.y));
	ImGui::PushStyleColor(ImGuiCol_Text, sender->userColor);
	ImGui::TextUnformatted(sender->userName.c_str());
	ImGui::PopStyleColor();
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(127,127,127,255));
	ImGui::TextUnformatted(timestamp.c_str());
	ImGui::PopStyleColor();
	ImGui::SetCursorPosX(pos.x);
}

Chat::ConnectionMessage::ConnectionMessage(bool connected, User* userIn, u64 tm, u64 id) : ChatMessage(userIn, tm, id), connect(connected)
{
}

void Chat::ConnectionMessage::Draw()
{
	ImVec2 pos = ImGui::GetCursorPos();
	ImGui::SetCursorPos(ImVec2(pos.x, pos.y));
	ImGui::PushStyleColor(ImGuiCol_Text, sender->userColor);
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
