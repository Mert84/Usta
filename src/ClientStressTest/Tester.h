#pragma once
#include <atomic>
#include <boost/asio.hpp>
#include <boost/asio/ssl/context.hpp>
#include "../UstaHttpClient/RequestGenerators.h"
#include "../UstaHttpClient/HttpClient.h"
#include "../UstaCommon/HttpRequest.h"
#include "../UstaCommon/HttpResponse.h"
#include "LinkProvider.h"
#include "date.h"
#include <chrono>
using namespace date;
using namespace std::chrono;

struct Tester
{
	LinkProvider _linkProvider;
	boost::asio::any_io_executor _executor;
	std::shared_ptr<boost::asio::ssl::context> _sslContext;
	int _concurrency = 0;
	int _numDownloads = 10;
	std::atomic_int _counter;

	explicit Tester(
		boost::asio::io_context& ioContext, 
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
	
	std::string DetermineLink()
	{
		return _linkProvider();
	}

	void Start()
	{
		_counter = 0;
		NextTest();
	}

	void NextTest()
	{
		++_counter;
		if (_counter < _numDownloads)
		{
			std::cout << "starting download no : " << _counter << std::endl;

			Test();
		}
		else
		{
			std::cout << "no more download necessary " << std::endl;

		}
	}

	void Test()
	{
		std::string link = DetermineLink();
		std::cout << "dowloading " << link << std::endl;

		HttpClientParameters parameters;
		parameters._executor = _executor;
		parameters._sslContext = _sslContext;

		auto httpClient = HttpClient::Make(std::move(parameters));
		

		ConnectionParameter connectionParameterBetter =
			make_connection_parameter(link);

		auto beforeConnect = std::chrono::steady_clock::now();
		httpClient->ConnectAsync(std::move(connectionParameterBetter),
			[httpClient, link, this, beforeConnect](std::error_code err) {
				if (err)
				{
					std::cout << "ConnectAsync failed" << err.message() << std::endl;
					return;
				}
				auto afterConnect = std::chrono::steady_clock::now();

				std::cout << "ConnectAsync succeeded in " <<
					duration_cast<milliseconds>(afterConnect - beforeConnect) << std::endl;

				HttpRequest request = make_get_request(link);

				httpClient->SendAsync(request,
					[this, afterConnect](std::error_code err, HttpResponse response) {
						if (err)
						{
							std::cout << "error occurred. Error message: " << err.message() << std::endl;
							return;
						}

						auto afterDownloadDone = std::chrono::steady_clock::now();

						std::cout << "Download succeeded in " <<
							duration_cast<milliseconds>(afterDownloadDone - afterConnect) << std::endl;

						if (Validate(response))
						{
							std::cout << "successfuly downloaded size " << response.Body.size() << std::endl;
							NextTest();
						}
						else
						{
							std::cout << "downloaded response is invalid : " << response.StatusCode() << std::endl;
						}
					});
			});
	}

	bool Validate(HttpResponse & response)
	{
		return 200 == response.StatusCode();
	}

};
