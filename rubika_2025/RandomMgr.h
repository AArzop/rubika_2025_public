#pragma once

#include <random>

class RandomMgr
{
public:
	RandomMgr();
	~RandomMgr();

	unsigned RandUInt(unsigned min, unsigned max);

	int RandInt32(int min, int max);

	double RandDouble(double min, double max);
	double RandNormalDouble(double center, double disp);

	void SetSeed(uint32_t seed);
	uint32_t GetSeed() const { return Seed; }

private:

	std::default_random_engine RandomEng;
	uint32_t Seed;
};