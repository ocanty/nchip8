//
// Created by ocanty on 17/09/18.
//


#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>

#include "nchip8.hpp"
#include "io.hpp"
#include "cpu_message.hpp"

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
    //
    if (m_args.size() < 2) // args should contain [executable,first_argument]
    {
        throw std::invalid_argument("No ROM! (Usage: nchip8 <path to rom>");
    }

    // try to read in the supplied rom file
    std::ifstream input_file(m_args[1], std::ios::binary | std::ios::in);

    if (!input_file) {
        throw std::invalid_argument("Could not open " + m_args[1] + "!");
    }

    // read in file
    std::vector<std::uint8_t> input_data;

    if(!input_file.eof() && !input_file.fail())
    {
        // seek to end of file, to get it's size
        input_file.seekg(0, std::ios_base::end);

        auto size = input_file.tellg();
        input_data.resize(size);

        // move file pointer back to beginning
        input_file.seekg(0, std::ios_base::beg);

        input_file.read((char*)&input_data[0], size);
    }


    m_cpu_daemon = std::make_shared<cpu_daemon>();
    m_gui = std::make_unique<gui>(m_cpu_daemon);

    if(m_args.size() > 2)
    {
        m_cpu_daemon->set_cpu_clockspeed(std::stoi(m_args.at(2)));
    }

    // reset the cpu
    m_cpu_daemon->send_message(cpu_message(cpu_message_type::Reset));

    // load rom
    m_cpu_daemon->send_message(cpu_message(
        cpu_message_type::LoadROM,
        input_data,
        [this]()
        {
            // tell cpu daemon to start doing cycles
            m_cpu_daemon->set_cpu_state(cpu_daemon::running);
        },

        []() {
            nchip8::log << "[nchip8] rom loading failed :(";
        }
    ));

    // start gui, note: blocking
    m_gui->loop();

    return 0;
}

}