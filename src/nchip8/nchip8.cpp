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

nchip8_app::nchip8_app(const std::vector<std::string> &args) :
    m_args(args)
{
    nchip8::log << "[nchip8] start" << '\n';
}

int nchip8_app::run()
{
     // complain if they don't supply a file
    if (m_args.size() < 2) // args should contain [executable,first_argument]
    {
        throw std::invalid_argument("No ROM! (Usage: nchip8 <path to rom>");
    }

    // try to read in the supplied rom file
    std::ifstream input_file(m_args[1], std::ios::binary | std::ios::in);

    if (!input_file)
    {
        throw std::invalid_argument("Could not open " + m_args[1] + "!");
    }

    // read in each byte
    std::vector<std::uint8_t> input_data;

    char byte;
    while (input_file >> byte)
    {
        input_data.push_back((std::uint8_t)byte);
    }

    nchip8::log << '\n';

    m_cpu_daemon = std::make_unique<cpu_daemon>();

    m_cpu_daemon->send_message(
        cpu_message(
            cpu_message_type::LoadROM,
            input_data,
            [this]()
            {
                // set the cpu running if the rom is loaded
                m_cpu_daemon->send_message(
                    cpu_message(cpu_message_type::SetStateRunning)
                );
            },

            []() {
                nchip8::log << "[nchip8] rom loading failed :(";
            }
        )
    );

    // set up clockspeed if specified
    if(m_args.size() > 2)
    {
        m_cpu_daemon->set_cpu_clockspeed(std::stoi(m_args.at(2)));
    }

    m_gui = std::make_unique<gui>(m_cpu_daemon);
    m_gui->loop(); // this is blocking

    return 0;
}

}