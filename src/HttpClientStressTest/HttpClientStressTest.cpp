#include <thread>
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include "StressTester.h"


std::shared_ptr<boost::asio::ssl::context> PrepareSslContext()
{
    using namespace boost::asio::ssl;
    auto sslContext = std::make_shared<context>(context::tlsv12);

    sslContext->load_verify_file("cacert.pem");
    sslContext->set_verify_mode(verify_peer);

    sslContext->set_verify_callback([](bool preverified, verify_context& verifyContext) {
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

void test()
{
    int concurrency = 100;
    int numberOfThreads = 10;

    //......
    boost::asio::io_context ioContext;
    std::cout << "started" << std::endl;

    {
        auto sslContext = PrepareSslContext();

        auto dummyWork = std::make_unique<boost::asio::io_context::work>(ioContext);

        StressTester stressTester(ioContext, sslContext, concurrency);
        stressTester();

        std::vector<std::thread> threads;
        for (int i = 0; i < numberOfThreads; i++)
        {
            threads.emplace_back([&ioContext]() {

                try
                {
                    ioContext.run();

                }
                catch (const std::exception& ex)
                {
                    std::cout << "exception occurred. Message : " << ex.what() << std::endl;
                }
                });
        }

        std::cout << "Press enter to stop the app" << std::endl;
        std::cin.get();

        stressTester.PrintResult();

        dummyWork.reset();
        ioContext.stop();

        for (auto & th : threads)
        {
            th.join();
        }
    }

    std::cout << "done." << std::endl;
}

int main()
{
    try
    {
        test();
    }
    catch (const std::exception & ex)
    {
        std::cout << "exception occurred during test. Exception message : " << ex.what() << std::endl;

    }

    return 0;
}
