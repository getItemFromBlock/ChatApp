#pragma once

#include <chrono>

// Default UDP timeout
#define UDP_TIMEOUT std::chrono::milliseconds(2000)

// Allow use of network simulator
#define NETWORK_SIMULATOR 0

// Allow use of mutexes in order to make functions thread safe
#define NETWORK_THREAD_SAFE 0

#ifdef _DEBUG

// Allow debugging network applications : when a client times out, consider its connection interrupted rather than disconnected so he can come back and resume it
#define NETWORK_INTERRUPTION 1

#endif // _DEBUG

#ifndef UDP_TIMEOUT
#define UDP_TIMEOUT std::chrono::seconds(1)
#endif
