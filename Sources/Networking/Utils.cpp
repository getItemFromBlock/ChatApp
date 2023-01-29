#include "Networking/Utils.hpp"
#include <numeric>

#include <assert.h>

namespace Networking::Utils
{
	bool IsSequenceNewer(uint16_t sNew, uint16_t sLast)
	{
		if (sNew == sLast)
			return false;
		return (sNew > sLast && sNew - sLast <= std::numeric_limits<uint16_t>::max() / 2) // cas simple : 4 > 2
			|| (sNew < sLast&& sLast - sNew > std::numeric_limits<uint16_t>::max() / 2); // dépassement : 2 > 65534
	}

	uint16_t SequenceDiff(uint16_t sNew, uint16_t sLast)
	{
		if (sNew == sLast)
			return 0;
		assert(IsSequenceNewer(sNew, sLast)); //!< S’ils ne sont pas dans le bon ordre le diff n’a pas beaucoup de sens et n’est pas permis
		if (sNew > sLast && sNew - sLast <= std::numeric_limits<uint16_t>::max() / 2)
			return sNew - sLast;
		//!< dépassement
		return (std::numeric_limits<uint16_t>::max() - sLast) + sNew + 1; //!< +1 pour compter le 0 : si sLast == sMax && sNew == 0, la difference est 1
	}

	std::chrono::milliseconds Now()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	}
}
