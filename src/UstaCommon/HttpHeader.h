#pragma once
#include <string>
#include <algorithm>
#include <vector>

struct StartLine
{
	std::vector<std::string> parts;
};

struct HeaderField
{
	std::string Name;
	std::string Value;
};

struct HttpHeader
{
	StartLine StartLine;
	void Add(HeaderField field)
	{
		Headers.push_back(std::move(field));
	}

	HeaderField * Get(const std::string & name)
	{
		auto loc = std::find_if(begin(Headers), end(Headers),
			[&name](const HeaderField& field) 
			{ return field.Name == name; });
		return ((end(Headers) == loc) ? nullptr : &(*loc));
	}

	std::vector<HeaderField> Headers;
};
