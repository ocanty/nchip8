#include <iostream>
#include <cstdint>
#include <thread>
#include <chrono>
#include <fstream>
#include <string>

#include "nchip8/nchip8.hpp"

int main(int argc, char** argv)
{
    std::vector<std::string> args;

    for(int i = 0; i < argc; i++)
    {
        args.emplace_back(std::string(argv[i]));
    }

    nchip8::nchip8_app app(args);
    std::this_thread::sleep_for(std::chrono::hours(99999));
    return 0;
}
