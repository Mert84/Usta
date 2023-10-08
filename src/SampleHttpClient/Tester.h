#pragma once
#include <boost/asio.hpp>
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

		ConnectionParameter connectionParameter;
		connectionParameter.host = "httpbin.org";
		connectionParameter.port = "80";

		httpClient->ConnectAsync(std::move(connectionParameter),
			[httpClient](std::error_code err) {
				if (err)
				{
					std::cout << "ConnectAsync failed" << std::endl;					
					return;
				}

				std::cout << "ConnectAsync succeeded" << std::endl;
				
				//HttpRequest request;
				std::string message = 
					"GET /get HTTP/1.1" "\r\n"
					"Host: httpbin.org" "\r\n"
					"\r\n"
					;

				httpClient->SendAsync(message,
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