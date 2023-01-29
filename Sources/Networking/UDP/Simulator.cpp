#include "Networking/UDP/Simulator.hpp"

#include <assert.h>
#include <iostream>

#include "Networking/Utils.hpp"

#if NETWORK_SIMULATOR
namespace Networking::UDP
{
	Simulator::Simulator()
	{
		// Initialize with a default seed
		seed(0);
	}
	void Simulator::seed(unsigned int value)
	{
		mDuplicateGenerator.seed(value);
		mLossGenerator.seed(value);
		mRandomDelayGenerator.seed(value);
		std::cout << "Set simulator seed to " << value << std::endl;
	}

	void Simulator::push(const Datagram& datagram, const Address& from)
	{
		assert(isEnabled());
		const bool isLost = std::uniform_int_distribution(1, 100)(mLossGenerator) < mLoss;
		if (isLost)
			return;

		using namespace std::chrono_literals;
		const auto now = Utils::Now();
		const bool shouldDuplicate = std::uniform_int_distribution(1, static_cast<int>(1000))(mDuplicateGenerator) < mDuplicateRate;
		const unsigned int nbCopies = shouldDuplicate ? std::uniform_int_distribution(0, (int)mDuplicate)(mDuplicateGenerator) : 1;
		PendingDatagram copy;
		copy.datagram = datagram;
		copy.from = from;
		copy.receivedTime = now;
		for (unsigned int i = 0; i < nbCopies; ++i)
		{
			copy.processTime = now + mFixedDelay * 1ms + std::uniform_int_distribution(static_cast<int>(0), static_cast<int>(mRandomDelay))(mRandomDelayGenerator) * 1ms;
			mPendingDatagrams.push_back(copy);
		}
	}
	std::vector<std::pair<Datagram, Address>> Simulator::poll()
	{
		std::stable_sort(mPendingDatagrams.begin(), mPendingDatagrams.end(), [](const PendingDatagram& left, const PendingDatagram& right) { return left.processTime < right.processTime; });

		auto firstNotToReturn = std::upper_bound(mPendingDatagrams.cbegin(), mPendingDatagrams.cend(), Utils::Now(), [](std::chrono::milliseconds now, const PendingDatagram& entry) { return now < entry.processTime; });
		std::vector<std::pair<Datagram, Address>> returned;
		returned.reserve(firstNotToReturn - mPendingDatagrams.cbegin());
		for (auto it = mPendingDatagrams.cbegin(); it != firstNotToReturn; ++it)
		{
			returned.emplace_back(it->datagram, it->from);
		}
		mPendingDatagrams.erase(mPendingDatagrams.begin(), firstNotToReturn);
		return returned;
	}
}
#endif
