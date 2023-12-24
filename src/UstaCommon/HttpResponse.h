#pragma once
#include <string>
#include "HttpHeader.h"

struct HttpResponse
{
	std::string Body;
	HttpHeader Header;
	int StatusCode()
	{
		if (Header.StartLine.parts.size() > 2 )
		{
			return std::stoi(Header.StartLine.parts[1]);
		}
		return -1;
	}
};