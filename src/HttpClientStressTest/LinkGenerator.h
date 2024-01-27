#pragma once
#include <random>
#include <string>
#include <vector>

struct LinkGenerator
{
	std::vector<std::string> _candidates =
	{
		"http://127.0.0.1/a.txt",
		"http://127.0.0.1/b.txt",
		"http://127.0.0.1/c.txt"
	};

	std::random_device _randomDevice;
	std::mt19937 _generator;
	std::uniform_int_distribution<> _distribution;


	LinkGenerator()
		:
		_generator(_randomDevice()),
		_distribution(0, static_cast<int>(_candidates.size()) - 1)
	{

	}

	std::string operator()()
	{
		int index = _distribution(_generator);
		return _candidates[index];
	}
};
