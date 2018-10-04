//
// Created by ocanty on 17/09/18.
//

#include "nchip8.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>

nchip8::nchip8(const std::vector<std::string>& args)
{
    // dump the arguments
    // std::cout << "[nchip8] start" << std::endl;
    for(auto& arg : args)
    {
        //std::cout << "[nchip8] arg: " << arg << std::endl;
    }

    // complain if they don't supply a file
    if(args.size() != 2) // args contains [executable,first_argument]
    {
        throw std::invalid_argument("No ROM! (Usage: nchip8 <path to rom>");
    }

    std::ifstream input_file(args[1], std::ios::binary | std::ios::in);

    if(!input_file)
    {
        throw std::invalid_argument("Could not open " + args[1] + "!");
    }

    std::vector<std::uint8_t> input_data;
    std::uint32_t byte_count = 0; // we use this to print the bytes in blocks of 16

    while(input_file.good() && !input_file.eof())
    {
        char byte = 0x00;
        input_file.read(&byte,sizeof(char));
        input_data.push_back((std::uint8_t)byte);

        byte_count++;
        byte_count%=16; // wrap in range of 0-16

        // we cast to int because passing a char to cout usually shows its ASCII representation
        // we want instead to keep the numeric view
        // we also remove the high bits if they're extended by the cast
        // std::cout << std::hex << std::setw(2) << std::setfill('0') << (0x000000FF & (std::uint32_t(byte))) << " ";

        // if the byte count has been reset, add a newline
        if(byte_count==0){ std::cout << std::endl;}
    }
    std::cout << std::endl;

    m_gui = std::make_unique<gui>();
    m_cpu_daemon = std::make_unique<cpu_daemon>();

    m_cpu_daemon->send_message(
        {
            cpu_daemon::cpu_message_type::LoadROM,
            input_data,
            []()
            {
                //std::cout << "Could not load ROM!" << std::endl;
                exit(-1); // ew
            }
        }
    );

    m_cpu_daemon->send_message(
        {
            cpu_daemon::cpu_message_type::SetStateRunning,
            {},
            []()
            {
                //std::cout << "you fucked up" << std::endl;
            }
        }
    );

}

nchip8::~nchip8()
{

}