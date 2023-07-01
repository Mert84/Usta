#pragma once
#include <memory>
#include <string>
#include "HttpClientParameters.h"
#include "../UstaCommon/HttpRequest.h"
#include "../UstaCommon/HttpResponse.h"

struct HttpClient
{
	static std::shared_ptr<HttpClient> Make(HttpClientParameters httpClientParameters)
	{
		return std::shared_ptr<HttpClient>(new HttpClient(std::move(httpClientParameters)));
	}

	template<typename Func>
	void SendAsync(const HttpRequest & request, Func func)
	{
		post(_httpClientParameters._executor, 
			[f = std::move(func)]() {

			HttpResponse response{};
			response.Body = "glad to know you";

			f(std::error_code(), std::move(response));
		});		
	}

private:
	explicit HttpClient(HttpClientParameters httpClientParameters)
		:
		_httpClientParameters(std::move(httpClientParameters))
	{		
	}

	HttpClientParameters _httpClientParameters;
};