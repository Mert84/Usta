#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl/context.hpp>
#include "../UstaHttpClient/RequestGenerators.h"
#include "../UstaHttpClient/HttpClient.h"
#include "../UstaCommon/HttpRequest.h"
#include "../UstaCommon/HttpResponse.h"
#include "LinkGenerator.h"

inline bool Validate(HttpResponse& response)
{
	if (response.StatusCode() == 200)
	{
		return true;
	}

	return false;
}

struct StressTester
{
	boost::asio::any_io_executor _executor;
	std::shared_ptr<boost::asio::ssl::context> _sslContext;
	int _concurrency = 0;
	int _succededCount = 0;
	int _failedCount = 0;
	std::mutex _countMtx;
	LinkGenerator _linkGenerator;

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

		std::string link = _linkGenerator();

		ConnectionParameter connectionParameterBetter =
			make_connection_parameter(link);

		auto beforeConnect = std::chrono::steady_clock::now();

		httpClient->ConnectAsync(std::move(connectionParameterBetter),
			[httpClient, link, this, beforeConnect](std::error_code err) {
				if (err)
				{
					std::lock_guard<std::mutex> lck(_countMtx);
					++_failedCount;
					std::cout << "ConnectAsync failed" << err.message() << std::endl;
					return;
				}
				
				auto afterConnect = std::chrono::steady_clock::now();
				auto connectMsec = 
					std::chrono::duration_cast<std::chrono::milliseconds>(afterConnect - beforeConnect);
				std::cout << "ConnectAsync succeeded in " << connectMsec .count() << " milliseconds" << std::endl;

				HttpRequest request = make_get_request(link);

				httpClient->SendAsync(request,
					[this, afterConnect](std::error_code err, HttpResponse response) {						

						if (err)
						{
							std::cout << "error occurred. Error message: " << err.message() << std::endl;
							return;
						}

						auto received = std::chrono::steady_clock::now();
						auto receiveTime =
							std::chrono::duration_cast<std::chrono::milliseconds>(received - afterConnect);
						std::cout << "Received in " << receiveTime.count() << " milliseconds" << std::endl;


						std::lock_guard<std::mutex> lck(_countMtx);

						auto valid = Validate(response);
						if (valid)
						{
							++_succededCount;
						}
						else
						{
							++_failedCount;
						}
					});
			});
	}

	void PrintResult()
	{
		std::cout 
			<< "sucess : " << _succededCount
			<< " fail : " << _failedCount
			<< std::endl;
	}

};