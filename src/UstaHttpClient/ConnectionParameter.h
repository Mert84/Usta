#pragma once
#include <string>
#include <algorithm>
#include <boost/url.hpp>

struct ConnectionParameter
{
	std::string host;
	std::string port;
	bool useTls = false;
};

inline std::string GuessPortForScheme(const std::string & scheme)
{
	static std::vector<std::pair<std::string, std::string>> mappings = {
		{ "http", "80" },
		{ "https", "443" },
		{ "ssl", "443" }
	};

	auto loc = std::find_if(begin(mappings), end(mappings), 
		[&scheme](std::pair<std::string, std::string>& item)
		{
			return item.first == scheme;
		});

	if (end(mappings) != loc)
	{
		return (*loc).second;
	}

	return std::string();
}

inline bool GuessTlsNeed(const std::string& scheme)
{
	static std::vector<std::pair<std::string, bool>> mappings = {
		{ "http", false },
		{ "https", true },
		{ "ssl", true }
	};

	auto loc = std::find_if(begin(mappings), end(mappings),
		[&scheme](std::pair<std::string, bool>& item)
		{
			return item.first == scheme;
		});

	if (end(mappings) != loc)
	{
		return (*loc).second;
	}

	return false;
}


inline ConnectionParameter make_connection_parameter(std::string link)
{
	auto parsedLink = boost::urls::parse_uri(link);

	ConnectionParameter connectionParameter;
	connectionParameter.host = parsedLink->host_address();
	connectionParameter.port = parsedLink->port();

	if (connectionParameter.port.empty())
	{
		connectionParameter.port = GuessPortForScheme(parsedLink->scheme());
	}
	connectionParameter.useTls = GuessTlsNeed(parsedLink->scheme());

	return connectionParameter;
}