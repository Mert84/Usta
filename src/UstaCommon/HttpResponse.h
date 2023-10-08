#pragma once
#include <string>
#include "HttpHeader.h"

struct HttpResponse
{
	int StatusCode = 400;
	std::string Body;
	HttpHeader Header;
};