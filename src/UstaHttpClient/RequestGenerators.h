#pragma once
#include "../UstaCommon/HttpRequest.h"
#include <boost/url.hpp>


HttpRequest make_get_request(std::string link)
{
	auto parsedLink = boost::urls::parse_uri(link);

	HttpRequest request;
	request._host = parsedLink->host_address();
	request._httpVerb = HttpVerb::GET;
	request._path = parsedLink->path();

	return request;
}
