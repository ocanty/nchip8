//
// Created by ocanty on 17/09/18.
//

#ifndef CHIP8_NCURSES_NCHIP8_HPP
#define CHIP8_NCURSES_NCHIP8_HPP

#include <memory>
#include <vector>

#include <bits/stdc++.h>

#include "io.hpp"
#include "cpu_daemon.hpp"
#include "gui.hpp"

namespace nchip8
{

class nchip8_app
{
public:
    //! @brief Constructor
    //! @param args Vector of string arguments
    nchip8_app(const std::vector<std::string> &args);

    //! @brief Destructor
    virtual ~nchip8_app() = default;

    //! @brief      Run the emulator application
    //! @returns    The return code for the process/application
    int run();
private:
    std::vector<std::string> m_args;

    std::unique_ptr<gui> m_gui;
    std::unique_ptr<cpu_daemon> m_cpu_daemon;
};

}

#endif //CHIP8_NCURSES_NCHIP8_HPP
