#pragma once
#include <string>
#include "HttpHeader.h"

struct HttpResponse
{
	std::string Body;
	HttpHeader Header;

	int StatusCode()
	{
		if (Header.StartLine.parts.size() <= 2)
		{
			return -1;
		}

		try
		{
			return std::stoi(Header.StartLine.parts[1]);
		}
		catch (const std::exception&)
		{
			return -2;
		}
	}
};