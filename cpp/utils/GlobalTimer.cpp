#include "../../header/utils/GlobalTimer.h"

GlobalTimer& GlobalTimer::instance_of()
{
	static GlobalTimer instance;
	return instance;
}
