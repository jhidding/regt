#pragma once
#include <iostream>
#include <chrono>

namespace System
{
    class Timer
    {
        std::string msg;
        std::chrono::time_point<std::chrono::system_clock> start;

        public:
            Timer(std::string const &msg):
                msg(msg), start(std::chrono::system_clock::now()) 
            {
                std::cerr << "Timing " << msg << " ... ";
            }

            double elapsed() const
            {
                auto stop = std::chrono::system_clock::now();
                std::chrono::duration<double> dt = stop - start;
                return dt.count();
            }
            
            void report() const
            {
                std::cerr << elapsed() << " seconds." << std::endl;
            }
    };
}

