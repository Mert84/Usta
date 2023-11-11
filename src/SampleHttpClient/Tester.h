#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl/context.hpp>
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
		parameters._sslContext = PrepareSslContext();

		auto httpClient = HttpClient::Make(std::move(parameters));

		std::string link = "https://httpbin.org/get";

		ConnectionParameter connectionParameterBetter = 
			make_connection_parameter(link);

		httpClient->ConnectAsync(std::move(connectionParameterBetter),
			[httpClient, link](std::error_code err) {
				if (err)
				{
					std::cout << "ConnectAsync failed"  << err.message()  << std::endl;
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

	std::shared_ptr<boost::asio::ssl::context> PrepareSslContext()
	{
		using namespace boost::asio::ssl;
		auto sslContext = std::make_shared<context>(context::tlsv12);

		sslContext->load_verify_file("cacert.pem");
		sslContext->set_verify_mode(verify_peer);

		sslContext->set_verify_callback([](bool preverified, verify_context & verifyContext) {			
			return preverified;
		});

		const char* defaultCipherList = "HIGH:!ADH:!MD5:!RC4:!SRP:!PSK:!DSS";
		auto ret = SSL_CTX_set_cipher_list(sslContext->native_handle(), defaultCipherList);

		auto options =
			boost::asio::ssl::context::default_workarounds |
			boost::asio::ssl::context::no_sslv2 |
			boost::asio::ssl::context::no_sslv3 |
			boost::asio::ssl::context::no_tlsv1 |
			boost::asio::ssl::context::no_tlsv1_1;
		sslContext->set_options(options);


		return sslContext;
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