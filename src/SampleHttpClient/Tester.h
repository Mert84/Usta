#pragma once
#include <boost/asio.hpp>

struct Tester
{
	boost::asio::io_context & _ioContext;
	explicit Tester(boost::asio::io_context & ioContext)
		:
		_ioContext(ioContext)
	{}

	void operator()()
	{

	}
};