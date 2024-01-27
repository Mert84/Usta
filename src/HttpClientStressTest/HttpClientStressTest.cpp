#include <thread>
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include "StressTester.h"

int main()
{
    boost::asio::io_context ioContext;
    std::cout << "started" << std::endl;

    {
        auto dummyWork = std::make_unique<boost::asio::io_context::work>(ioContext);

        StressTester stressTester(ioContext);
        stressTester();

        std::thread th([&ioContext]() {

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


        std::cout << "Press enter to stop the app" << std::endl;
        std::cin.get();

        dummyWork.reset();
        ioContext.stop();

        th.join();
    }

    std::cout << "done." << std::endl;
}
