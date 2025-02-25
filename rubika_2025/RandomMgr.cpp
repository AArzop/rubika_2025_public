#include "RandomMgr.h"

RandomMgr::RandomMgr() : Seed(0)
{
}

RandomMgr::~RandomMgr()
{
}

unsigned RandomMgr::RandUInt(unsigned min, unsigned max)
{
	std::uniform_int_distribution<uint32_t> uid(min, max);
	return uid(RandomEng);
}

int RandomMgr::RandInt32(int min, int max)
{
	std::uniform_int_distribution<int32_t> uid(min, max);
	return uid(RandomEng);
}

double RandomMgr::RandDouble(double min, double max)
{
	std::uniform_real_distribution<double> urd(min, max);
	return urd(RandomEng);
}

double RandomMgr::RandNormalDouble(double center, double disp)
{
	std::normal_distribution<double> nrd(center, disp);
	return nrd(RandomEng);
}

void RandomMgr::SetSeed(uint32_t seed)
{
	RandomEng.seed(seed);
	Seed = seed;
}
