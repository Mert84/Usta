#pragma once
#include <boost/asio.hpp>
#include "../UstaHttpClient/RequestGenerators.h"
#include "../UstaHttpClient/HttpClient.h"
#include "../UstaCommon/HttpRequest.h"
#include "../UstaCommon/HttpResponse.h"

struct Tester
{
	boost::asio::any_io_executor _executor;

	explicit Tester(boost::asio::io_context & ioContext)
		:
		_executor(ioContext.get_executor())
	{}

	void operator()()
	{
		HttpClientParameters parameters;
		parameters._executor = _executor;

		auto httpClient = HttpClient::Make(std::move(parameters));

		std::string link = "http://httpbin.org/get";

		ConnectionParameter connectionParameterBetter = 
			make_connection_parameter(link);

		httpClient->ConnectAsync(std::move(connectionParameterBetter),
			[httpClient, link](std::error_code err) {
				if (err)
				{
					std::cout << "ConnectAsync failed" << std::endl;					
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

						std::cout << "Request succeeded. " << std::endl << std::endl;

						Print(response);						
					});
			});
	}

	static void Print(const HttpResponse& response)
	{
		for (auto& startLinePart : response.Header.StartLine.parts)
		{
			std::cout << startLinePart << " ";
		}
		std::cout << std::endl;

		for (auto& header : response.Header.Headers)
		{
			std::cout << header.Name << ": " << header.Value << std::endl;
		}

		std::cout << std::endl << response.Body;
	}
};