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
#include <boost/asio/ssl.hpp>

struct HttpClient : std::enable_shared_from_this<HttpClient>
{
	static std::shared_ptr<HttpClient> Make(HttpClientParameters httpClientParameters)
	{
		return std::shared_ptr<HttpClient>(new HttpClient(std::move(httpClientParameters)));
	}

	template<typename Callable>
	void ConnectAsync(ConnectionParameter connectionParameter, Callable callable)
	{
		_useTls = connectionParameter.useTls;
		_resolver.async_resolve(
			{ connectionParameter.host, connectionParameter.port },
			[callabl = std::move(callable), 
			sharedThis = this->shared_from_this(), 
			this,
			host = connectionParameter.host]
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

				if (_useTls)
				{
					if (!SSL_set_tlsext_host_name(_tlsSocket->native_handle(), host.c_str()))
					{
						callabl(std::make_error_code(static_cast<std::errc>(static_cast<int>(::ERR_get_error()))));
						return;
					}

					boost::asio::async_connect(
						_tlsSocket->lowest_layer(),
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

						_tlsSocket->async_handshake(boost::asio::ssl::stream_base::client,
							[sharedThis = std::move(sharedThis), 
							this,
							callbl = std::move(callbl)]
							(boost::system::error_code err)
							{
								callbl(err);
								DeferDeletion();
							});
					}
					);
				}
				else
				{
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
			}
		);
	}

	template<typename Callable>
	void SendAsync(const HttpRequest & httpRequest, Callable callable)
	{

		ReadResponseAsync(std::move(callable));
		SendMessageAsync(HttpRequestSerializer{ httpRequest }());
	}

private:
	explicit HttpClient(HttpClientParameters httpClientParameters)
		:
		_httpClientParameters(std::move(httpClientParameters)),
		_resolver(_httpClientParameters._executor),
		_socket(_httpClientParameters._executor),
		_httpResponseStreamParser(&_httpResponsePopulator)
	{
		if (_httpClientParameters._sslContext)
		{
			using namespace boost::asio;
			_tlsSocket = std::make_shared<ssl::stream<ip::tcp::socket>>(
				_httpClientParameters._executor,
				*_httpClientParameters._sslContext);
		}
	}

	template<typename Callable>
	void ReadResponseAsync(Callable callable)
	{
		if (_useTls)
		{
			if (_tlsSocket)
			{
				ReadResponseAsyncT(*_tlsSocket, std::move(callable));
			}
		}
		else
		{
			ReadResponseAsyncT(_socket, std::move(callable));
		}
	}

	template<typename Callable, typename SocketType>
	void ReadResponseAsyncT(SocketType& socket, Callable callable)
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

				callable(std::error_code(), std::move(_httpResponsePopulator.TheResponse()));
				DeferDeletion();
			});
	}

	void SendMessageAsync(const std::string & content)
	{
		if (_useTls)
		{
			if (_tlsSocket)
			{
				SendMessageAsyncT(*_tlsSocket, content);
			}
		}
		else
		{
			SendMessageAsyncT(_socket, content);
		}		
	}

	template<typename SocketType>
	void SendMessageAsyncT(SocketType & socket, const std::string& content)
	{
		std::ostream os(&_request);
		os << content;

		boost::asio::async_write(socket, _request,
			[sharedThis = this->shared_from_this()]
		(boost::system::error_code err, std::size_t) {
			//TODO: handle error case where we fail to send 
			std::cout << "written : " << err.message() << std::endl;
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
	std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> _tlsSocket;

	boost::asio::streambuf _request;
	boost::asio::streambuf _response;

	HttpResponsePopulator _httpResponsePopulator;
	HttpResponseStreamParser _httpResponseStreamParser;

	bool _useTls = false;
};