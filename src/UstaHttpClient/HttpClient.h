#pragma once
#include <memory>
#include <string>
#include "HttpClientParameters.h"
#include "ConnectionParameter.h"
#include "../UstaCommon/HttpRequest.h"
#include "../UstaCommon/HttpResponse.h"
#include "../UstaCommon/HttpRequestSerializer.h"
#include "HttpResponseStreamParser.h"
#include "HttpResponsePopulator.h"

struct HttpClient : std::enable_shared_from_this<HttpClient>
{
	static std::shared_ptr<HttpClient> Make(HttpClientParameters httpClientParameters)
	{
		return std::shared_ptr<HttpClient>(new HttpClient(std::move(httpClientParameters)));
	}

	template<typename Callable>
	void ConnectAsync(ConnectionParameter connectionParameter, Callable callable)
	{
		_resolver.async_resolve(
			{ connectionParameter.host, connectionParameter.port },
			[callabl = std::move(callable), 
			sharedThis = this->shared_from_this(), 
			this]
			(boost::system::error_code err,
			const boost::asio::ip::tcp::resolver::results_type& endpoints) mutable
			{
				if (err)
				{
					std::cout << "resolve error occurred : " << err.message() << std::endl;
					callabl(err);
					DeferDeletion();
					return;
				}

				boost::asio::async_connect(
					_socket,
					endpoints,
					[callbl = std::move(callabl),
					sharedThis = std::move(sharedThis),
					this]
					(boost::system::error_code err, boost::asio::ip::tcp::endpoint /*ep*/)
					{
						if (err)
						{
							std::cout << "connect error occurred : " << err.message() << std::endl;
							callbl(err);
							DeferDeletion();
							return;
						}

						callbl(std::error_code());
						DeferDeletion();
					}
				);
			}
		);
	}

	template<typename Callable>
	void SendAsync(const HttpRequest & httpRequest, Callable callable)
	{
		ReadResponseAsync(_socket, std::move(callable));
		SendMessageAsync(_socket, HttpRequestSerializer{ httpRequest }());
	}

private:
	explicit HttpClient(HttpClientParameters httpClientParameters)
		:
		_httpClientParameters(std::move(httpClientParameters)),
		_resolver(_httpClientParameters._executor),
		_socket(_httpClientParameters._executor),
		_httpResponseStreamParser(&_httpResponsePopulator)
	{
	}

	template<typename Callable>
	void ReadResponseAsync(boost::asio::ip::tcp::socket& socket, Callable callable)
	{

		boost::asio::async_read_until(socket, _response, _httpResponseStreamParser,
			[this,
			sharedThis = this->shared_from_this(),
			callable = std::move(callable)]
			(boost::system::error_code err, size_t byteCount) mutable{
				if (err)
				{
					callable(err, HttpResponse{});
					return;
				}
				
				//std::string data{ std::istreambuf_iterator<char>(&_response), std::istreambuf_iterator<char>() };				

				callable(std::error_code(), std::move(_httpResponsePopulator.TheResponse()));
			});
	}

	void SendMessageAsync(boost::asio::ip::tcp::socket & socket, const std::string & content)
	{
		std::ostream os(&_request);
		os << content;
		
		boost::asio::async_write(socket, _request, 
			[sharedThis = this->shared_from_this()]
		(boost::system::error_code err, std::size_t) {
			//TODO: handle error case where we fail to send 
		});
	}

	void DeferDeletion()
	{
		post(_httpClientParameters._executor,
			[sharedThis = this->shared_from_this()]() mutable {
			sharedThis.reset();
		});
	}

	HttpClientParameters _httpClientParameters;
	boost::asio::ip::tcp::resolver _resolver;
	boost::asio::ip::tcp::socket _socket;
	boost::asio::streambuf _request;
	boost::asio::streambuf _response;

	HttpResponsePopulator _httpResponsePopulator;
	HttpResponseStreamParser _httpResponseStreamParser;
};