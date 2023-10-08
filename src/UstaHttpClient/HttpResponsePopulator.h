#pragma once

#include <boost/asio.hpp>
#include <utility>
#include <algorithm>
#include "../UstaCommon/HttpHeader.h"
#include "HttpHeaderParser.h"

enum class ParserState
{
	waiting_for_headers,
	waiting_for_body,
	done
};

enum class TransferMethod
{
	content_length,
	chunked,
	none
};

struct HttpResponsePopulator
{
	typedef boost::asio::buffers_iterator<
		boost::asio::streambuf::const_buffers_type> Iterator;

	const bool done = true;
	const bool notdone = false;
	ParserState _state = ParserState::waiting_for_headers;
	std::string _buffer;
	std::string _doubleNewLines = "\r\n\r\n";
	HttpResponse _httpResponse;
	TransferMethod _transferMethod = TransferMethod::none;
	size_t _expectedMessageSize = 0;
	size_t _beginningOfBodyIndex;

	HttpResponse & TheResponse()
	{
		return _httpResponse;
	}

	template <typename Iterator>
	std::pair<Iterator, bool> operator()(
		Iterator beginItr, Iterator endItr)
	{
		if (ParserState::done == _state)
		{
			return std::make_pair(beginItr, done);
		}

		_buffer.insert(_buffer.end(), beginItr, endItr);

		if (ParserState::waiting_for_headers == _state)
		{
			auto loc = std::search(begin(_buffer), end(_buffer), begin(_doubleNewLines), end(_doubleNewLines));
			if (end(_buffer) == loc)
			{
				return std::make_pair(endItr, notdone);
			}

			std::string headerData{ begin(_buffer), loc };
			
			auto possibleHeader = HttpHeaderParser{}(headerData);
			if (!possibleHeader.first)
			{
				_state = ParserState::done;
				return std::make_pair(endItr, done);				
			}

			_httpResponse.Header = std::move(possibleHeader.second);
			auto& httpHeader = _httpResponse.Header;

			_transferMethod = DetermineTransferMethod(httpHeader);
			if (TransferMethod::none == _transferMethod)
			{
				_state = ParserState::done;
				return std::make_pair(endItr, done);
			}
			if (TransferMethod::chunked == _transferMethod)
			{
				//TODO: support chunked transfer type
				_state = ParserState::done;
				return std::make_pair(endItr, done);
			}

			_state = ParserState::waiting_for_body;
			size_t bodySize;
			try
			{
				bodySize = (size_t)std::stoi(httpHeader.Get("Content-Length")->Value);
			}
			catch (const std::exception&)
			{
				_state = ParserState::done;
				return std::make_pair(endItr, done);
			}

			_expectedMessageSize = headerData.size() + _doubleNewLines.size() + bodySize;
			_beginningOfBodyIndex = headerData.size() + _doubleNewLines.size();
			if (_expectedMessageSize > _buffer.size())
			{
				return std::make_pair(endItr, notdone);
			}

			auto beginningOfBody = begin(_buffer) + _beginningOfBodyIndex;
			auto endOfBody = begin(_buffer) + _expectedMessageSize;

			_state = ParserState::done;

			_httpResponse.Body = std::string(beginningOfBody, endOfBody);
			return std::make_pair(endItr, done);
		}		
		else if(ParserState::waiting_for_body == _state)
		{
			if (_expectedMessageSize > _buffer.size())
			{
				return std::make_pair(endItr, notdone);
			}

			_state = ParserState::done;
			auto beginningOfBody = begin(_buffer) + _beginningOfBodyIndex;
			auto endOfBody = begin(_buffer) + _expectedMessageSize;
			_httpResponse.Body = std::string(beginningOfBody, endOfBody);
			return std::make_pair(endItr, done);
		}
		
		//never reach here
		return std::make_pair(endItr, done);
	}
	
	TransferMethod DetermineTransferMethod(HttpHeader & header)
	{
		auto encoding = header.Get("Transfer-Encoding");
		if (encoding && encoding->Value == "chunked")
		{
			return TransferMethod::chunked;
		}

		auto contentLength = header.Get("Content-Length");
		if (contentLength && !contentLength->Value.empty())
		{
			return TransferMethod::content_length;
		}

		return TransferMethod::none;
	}
};
