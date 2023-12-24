#include <thread>
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include "Tester.h"
#include <boost/asio/ssl/context.hpp>

std::shared_ptr<boost::asio::ssl::context> PrepareSslContext()
{
    using namespace boost::asio::ssl;
    auto sslContext = std::make_shared<context>(context::tlsv12);

    try
    {
        sslContext->load_verify_file("cacert.pem");

    }
    catch (const std::exception& ex)
    {
        std::cout << "cannot load cacert.pem. exception message : " << ex.what() << std::endl;

    }
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

int main()
{
    int numThreads = 1;
    int concurrency = 1;

    //
    boost::asio::io_context ioContext;
    std::cout << "started" << std::endl;
    auto sslContext = PrepareSslContext();

    {
        auto dummyWork = std::make_unique<boost::asio::io_context::work>(ioContext);

        Tester tester(ioContext, sslContext, concurrency);
        tester();

        std::vector<std::thread> threads;

        for (int i = 0; i < numThreads; ++i)
        {
            threads.emplace_back([&ioContext]() {

                try
                {
                    ioContext.run();
                    std::cout << "ioContext is out of work" << std::endl;

                }
                catch (const std::exception& ex)
                {
                    std::cout << "exception occurred. Message : " << ex.what() << std::endl;
                }
            });
        }



        std::cout << "Press enter to stop the app" << std::endl;
        std::cin.get();

        dummyWork.reset();
        ioContext.stop();

        for (auto & th : threads)
        {
            th.join();
        }
    }

    std::cout << "done." << std::endl;
}