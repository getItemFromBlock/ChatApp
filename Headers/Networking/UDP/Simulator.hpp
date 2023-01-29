#pragma once

#include <random>

#include "Networking/NetworkSettings.hpp"
#include "Datagram.hpp"
#include "Networking/Address.hpp"
#include "Core/Types.hpp"

#if NETWORK_SIMULATOR
namespace Networking::UDP
{
	class Simulator
	{
	public:
		Simulator();

		void enable() { mEnabled = true; }
		void disable() { mEnabled = false; }
		bool isEnabled() const { return mEnabled; }
		// Initialize random generators with the given seed, useful for tests especially
		void seed(unsigned int value);

		void setDuplicate(u8 duplicate) { mDuplicate = duplicate; }
		void setLossRate(u8 loss) { mLoss = loss; }
		void setDelay(u8 fixed, u8 random) { mFixedDelay = fixed; mRandomDelay = random; }

		// Push datagram to the simulator to store them and apply delay etc.
		void push(const Datagram& datagram, const Address& from);
		// Retrieve datagrams ready to be processed. Can return more datagrams than pushed as they can be duplicated.
		std::vector<std::pair<Datagram, Address>> poll();

	private:
		struct PendingDatagram {
			Datagram datagram;
			Address from;
			std::chrono::milliseconds receivedTime;
			std::chrono::milliseconds processTime;
		};
		std::vector<PendingDatagram> mPendingDatagrams;

		u8 mDuplicate = 3;
		u16 mDuplicateRate = 1; // This value should remain small in normal usage as that leads to duplication which can take a lot of extra memory and poor performances
		std::default_random_engine mDuplicateGenerator;
		u8 mLoss = 5;
		std::default_random_engine mLossGenerator;
		u8 mFixedDelay = 0;
		u8 mRandomDelay = 50;
		std::default_random_engine mRandomDelayGenerator;
		bool mEnabled = false;
	};
}
#endif