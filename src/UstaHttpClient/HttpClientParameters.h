#pragma once
#include <boost/asio.hpp>

struct HttpClientParameters
{
	boost::asio::any_io_executor _executor;
	std::shared_ptr<boost::asio::ssl::context> _sslContext;
};
