//
// Created by ocanty on 17/09/18.
//

#include "nchip8.hpp"
#include "io.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>

namespace nchip8
{

nchip8_app::nchip8_app(const std::vector<std::string> &args)
{
    // dump the arguments
    nchip8::log << "[nchip8] start" << '\n';

    for (auto &arg : args)
    {
        nchip8::log << "[nchip8] arg: " << arg << '\n';
    }

    // complain if they don't supply a file
    if (args.size() < 2) // args contains [executable,first_argument]
    {
        throw std::invalid_argument("No ROM! (Usage: nchip8 <path to rom>");
    }

    std::ifstream input_file(args[1], std::ios::binary | std::ios::in);

    if (!input_file)
    {
        throw std::invalid_argument("Could not open " + args[1] + "!");
    }

    std::vector<std::uint8_t> input_data;
    std::uint32_t byte_count = 0; // we use this to print the bytes in blocks of 16

    while (input_file.good() && !input_file.eof())
    {
        char byte = 0x00;
        input_file.read(&byte, sizeof(char));
        input_data.push_back((std::uint8_t)byte);

        byte_count++;
        byte_count %= 16; // wrap in range of 0-16

        // we cast to int because passing a char to cout usually shows its ASCII representation
        // we want instead to keep the numeric view
        // we also remove the high bits if they're extended by the cast
        nchip8::log << std::hex << std::setw(2) << std::setfill('0') << (0x000000FF & (std::uint32_t(byte))) << " ";

        // if the byte count has been reset, add a newline
        if (byte_count == 0)
        {
            nchip8::log << '\n';
        }
    }

    nchip8::log << '\n';

    m_cpu_daemon = std::make_shared<cpu_daemon>();

    m_cpu_daemon->send_message({
        cpu_daemon::cpu_message_type::LoadROM,
        input_data,
        []()
        {
            nchip8::log << "Could not load ROM!" << '\n';
            exit(-1); // ew
        }}
    );

    m_cpu_daemon->send_message(
        {
            cpu_daemon::cpu_message_type::SetStateRunning,
            {},
            []()
            {
                
            }
        }
    );

    if(args.size() > 2)
    {
        m_cpu_daemon->set_cpu_clockspeed(std::stoi(args.at(2)));
    }

    m_gui = std::make_unique<gui>(m_cpu_daemon);
    m_gui->loop(); // this is blocking

}

}