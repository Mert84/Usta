#pragma once
#include <random>
#include <vector>
#include <string>

struct LinkProvider
{
	std::vector<std::string> _links =
	{
		"https://httpbin.org/get" ,
		"http://127.0.0.1/a.txt" ,
		"http://localhost/a.txt" ,
		"http://127.0.0.1/index.html"
	};
	std::random_device _randomDevice;  // a seed source for the random number engine
	std::mt19937 _generator; // mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> _distribution;



	LinkProvider()
		:
		_generator(_randomDevice()),
		_distribution(0, static_cast<int>(_links.size()) - 1)
	{
	}

	std::string operator()()
	{
		return _links[_distribution(_generator)];
	}
};