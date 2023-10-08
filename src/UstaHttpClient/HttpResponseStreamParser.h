#pragma once
#include <boost/asio.hpp>
#include <utility>
#include "HttpResponsePopulator.h"

struct HttpResponseStreamParser
{
	HttpResponseStreamParser(HttpResponsePopulator * populator)
		:
		_populator(populator)
	{
	}

	typedef boost::asio::buffers_iterator<
		boost::asio::streambuf::const_buffers_type> Iterator;

	template <typename Iterator>
	std::pair<Iterator, bool> operator()(Iterator beginItr, Iterator endItr)
	{
		return (*_populator)(beginItr, endItr);
	}

	HttpResponsePopulator * _populator = nullptr;
};

namespace boost
{
	namespace asio
	{
		template <> struct is_match_condition<HttpResponseStreamParser>
		: public boost::true_type
		{
		};
	}
}