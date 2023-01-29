#include "Networking/Errors.hpp"

namespace Networking::Sockets
{
	int GetError()
	{
#ifdef _WIN32
		return WSAGetLastError();
#else
		return errno;
#endif
	}
	Errors GetErrorCasted()
	{
		return static_cast<Errors>(GetError());
	}
}