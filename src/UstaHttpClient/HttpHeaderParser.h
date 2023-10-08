#pragma once
#include "../UstaCommon/HttpHeader.h"
#include <string>
#include <tuple>
#include <boost/algorithm/string.hpp>

struct HttpHeaderParser
{
	std::pair<bool, HttpHeader> operator()(const std::string & data)
	{
		std::vector<std::string> lines;
		boost::split(lines, data, boost::is_any_of("\r\n"), boost::token_compress_on);
		if (lines.empty())
		{
			return std::make_pair<bool, HttpHeader>(false, HttpHeader{});
		}

		auto& startLine = lines[0];
		
		HttpHeader httpHeader;
		boost::split(httpHeader.StartLine.parts, startLine, boost::is_any_of(" "));
		if (httpHeader.StartLine.parts.size() < 3)
		{
			return std::make_pair<bool, HttpHeader>(false, HttpHeader{});
		}

		lines.erase(lines.begin());

		for (auto & line : lines)
		{
			httpHeader.Add(ParseHeaderLine(line));
		}

		return std::make_pair<bool, HttpHeader>(true, std::move(httpHeader));
	}

	HeaderField ParseHeaderLine(const std::string& headerLine)
	{
		HeaderField headerField;

		auto loc = std::find(begin(headerLine), end(headerLine), ':');
		if (end(headerLine) == loc)
		{
			headerField.Name = headerLine;
		}
		else
		{
			headerField.Name = std::string(begin(headerLine), loc);
			headerField.Value = std::string(++loc, end(headerLine));
		}

		boost::algorithm::trim(headerField.Name);
		boost::algorithm::trim(headerField.Value);

		return headerField;
	}
};