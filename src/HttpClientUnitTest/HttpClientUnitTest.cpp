#define CATCH_CONFIG_MAIN
#include "../../thirdparty/include/catch2/catch_amalgamated.hpp"
#include "../UstaHttpClient/RequestGenerators.h"
#include "../UstaHttpClient/ConnectionParameter.h"
#include <iostream>

TEST_CASE("Http Connection Parameters Parse")
{
	auto parameter1 = make_connection_parameter("http://whatever.com/getrequest/path");
	CHECK(parameter1.host == "whatever.com");
	CHECK(parameter1.port == "80");


	auto parameterWithExplicitPort = make_connection_parameter("http://whatever.com:9591/getrequest/path");
	CHECK(parameterWithExplicitPort.port == "9591");

	auto parameterWithHttps = make_connection_parameter("https://whatever.com/getrequest/path");
	CHECK(parameterWithHttps.port == "443");
}


TEST_CASE("Get Request Generator Parsing")
{
	auto getRequest1 = make_get_request("http://httpbin.org/get");
	CHECK(getRequest1._host == "httpbin.org");
	CHECK(getRequest1._httpVerb == HttpVerb::GET);
	CHECK(getRequest1._path == "/get");


	auto getRequestWithMultiplePathParts = make_get_request("http://whatever.com:9591/getrequest/path");
	CHECK(getRequestWithMultiplePathParts._path == "/getrequest/path");
}
