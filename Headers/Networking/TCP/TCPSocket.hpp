#pragma once

#include "Networking/Sockets.hpp"

#include <string>
#include <vector>

namespace Networking::TCP
{
	class TCPSocket
	{
	public:
		TCPSocket();
		~TCPSocket();

		bool Connect(const std::string& ipaddress, unsigned short port);
		bool Send(const unsigned char* data, unsigned short len);
		bool Receive(std::vector<unsigned char>& buffer);

	private:
		SOCKET mSocket;
	};
}