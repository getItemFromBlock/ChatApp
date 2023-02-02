#include "Chat/User.hpp"

namespace Chat
{

	User::User(const char* name, u64 id, const Resources::Texture* texIn)
	{
		userName = name;
		userID = id;
		userTex = texIn;
	}

	User::~User()
	{
	}
}