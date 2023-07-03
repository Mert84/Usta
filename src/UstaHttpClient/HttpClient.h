#pragma once
#include <memory>
#include <string>
#include "HttpClientParameters.h"
#include "ConnectionParameter.h"
#include "../UstaCommon/HttpRequest.h"
#include "../UstaCommon/HttpResponse.h"

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
			[callabl = std::move(callable), sharedThis = this->shared_from_this(), this]
			(boost::system::error_code err,
			const boost::asio::ip::tcp::resolver::results_type& endpoints) mutable
			{
				if (err)
				{
					std::cout << "resolve error occurred : " << err.message() << std::endl;
					callabl(err);
					return;
				}

				boost::asio::async_connect(
					_socket,
					endpoints,
					[callbl = std::move(callabl),
					sharedThis = std::move(sharedThis)](boost::system::error_code err, boost::asio::ip::tcp::endpoint /*ep*/)
					{
						if (err)
						{
							std::cout << "connect error occurred : " << err.message() << std::endl;
							callbl(err);
							return;
						}

						callbl(std::error_code());
					}
				);
			}
		);
	}

	template<typename Callable>
	void SendAsync(const HttpRequest & request, Callable callable)
	{
		post(_httpClientParameters._executor, 
			[callabl = std::move(callable),
			sharedThis = this->shared_from_this()]() mutable {

			HttpResponse response{};
			response.Body = "glad to know you";

			callabl(std::error_code(), std::move(response));
		});
	}

private:
	explicit HttpClient(HttpClientParameters httpClientParameters)
		:
		_httpClientParameters(std::move(httpClientParameters)),
		_resolver(_httpClientParameters._executor),
		_socket(_httpClientParameters._executor)
	{		
	}

	HttpClientParameters _httpClientParameters;
	boost::asio::ip::tcp::resolver _resolver;
	boost::asio::ip::tcp::socket _socket;
};