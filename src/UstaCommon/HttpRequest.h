#pragma once

enum class HttpVerb
{
	GET,
	POST
};

inline std::string to_string(HttpVerb httpVerb)
{
	switch (httpVerb)
	{
	case HttpVerb::GET:
		return "GET";
	case HttpVerb::POST:
		return "POST";
	default:
		return "";
	}
}

struct HttpRequest
{
	HttpVerb _httpVerb;
	std::string _path;
	std::string _host;
};

