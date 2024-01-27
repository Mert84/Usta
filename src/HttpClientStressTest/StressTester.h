#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl/context.hpp>
#include "../UstaHttpClient/RequestGenerators.h"
#include "../UstaHttpClient/HttpClient.h"
#include "../UstaCommon/HttpRequest.h"
#include "../UstaCommon/HttpResponse.h"

inline bool Validate(HttpResponse& response)
{
	if (response.StatusCode() == 200)
	{
		if (response.Body == "asdasd")
		{
			return true;
		}
	}

	return false;
}

struct StressTester
{
	boost::asio::any_io_executor _executor;
	std::shared_ptr<boost::asio::ssl::context> _sslContext;
	int _concurrency = 0;

	explicit StressTester(boost::asio::io_context & ioContext,
		std::shared_ptr<boost::asio::ssl::context> sslContext,
		int concurrency)
		:
		_executor(ioContext.get_executor()),
		_sslContext(move(sslContext)),
		_concurrency(concurrency)
	{}

	void operator()()
	{
		for (int i = 0; i < _concurrency; i++)
		{
			Start();
		}
	}

	void Start()
	{
		HttpClientParameters parameters;
		parameters._executor = _executor;
		parameters._sslContext = _sslContext;

		auto httpClient = HttpClient::Make(std::move(parameters));

		std::string link = "http://127.0.0.1/a.txt";

		ConnectionParameter connectionParameterBetter =
			make_connection_parameter(link);

		httpClient->ConnectAsync(std::move(connectionParameterBetter),
			[httpClient, link](std::error_code err) {
				if (err)
				{
					std::cout << "ConnectAsync failed" << err.message() << std::endl;
					return;
				}

				std::cout << "ConnectAsync succeeded" << std::endl;

				HttpRequest request = make_get_request(link);

				httpClient->SendAsync(request,
					[](std::error_code err, HttpResponse response) {
						if (err)
						{
							std::cout << "error occurred. Error message: " << err.message() << std::endl;
							return;
						}


						auto valid = Validate(response);
						if (valid)
						{
							std::cout << "Request succeeded. " << std::endl;
						}
						else
						{
							std::cout << "Request failed. " << std::endl;
						}
					});
			});
	}
};