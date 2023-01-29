#include "Networking/Network.hpp"

#include "Networking/Sockets.hpp"
#include "Networking/Errors.hpp"

#include <iostream>

namespace Networking
{
	Network::Network()
	{
		if (!Networking::Start())
		{
			std::cout << "Network Error : " << Networking::Sockets::GetError() << std::endl;
		}
		else
		{
			isValid = true;
		}
	}

	Network::~Network()
	{
		if (isValid)
		{
			Networking::Release();
		}
	}
}