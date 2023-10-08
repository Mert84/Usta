#pragma once
#include <string>
#include <sstream>
#include "HttpRequest.h"

struct HttpRequestSerializer
{
	HttpRequestSerializer(const HttpRequest & httpRequest)
		:
		_httpRequest(httpRequest)
	{
	}

	std::string operator()()
	{
		std::stringstream ss;

		ss
			<< to_string(_httpRequest._httpVerb) << " " << _httpRequest._path << " HTTP/1.1\r\n"
			<< "Host: " << _httpRequest._host << "\r\n\r\n";
		
		return ss.str();
	}

private:
	const HttpRequest& _httpRequest;

};