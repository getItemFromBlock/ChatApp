#pragma once

#include <vector>

#include "Core/Types.hpp"

namespace Chat
{

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
		FILE_DATA,
	};

	class ActionData
	{
	public:
		ActionData() = default;
		ActionData(Action typeIn, const u8* dataIn, u64 szIn) : type(typeIn), data(dataIn, dataIn + szIn) { }

		~ActionData() = default;

		Chat::Action type = Action::PING;
		std::vector<u8> data;
	};
}