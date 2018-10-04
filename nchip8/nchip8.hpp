//
// Created by ocanty on 17/09/18.
//

#ifndef CHIP8_NCURSES_NCHIP8_HPP
#define CHIP8_NCURSES_NCHIP8_HPP

#include <memory>
#include <vector>
#include "cpu_daemon.hpp"
#include "gui.hpp"

class nchip8
{
public:
    nchip8(const std::vector<std::string>& args);
    ~nchip8();

private:
//    //! @brief Constructs m_gui & m_cpu_daemon
//    void modules_init();

    std::unique_ptr<gui> m_gui;
    std::unique_ptr<cpu_daemon> m_cpu_daemon;
};


#endif //CHIP8_NCURSES_NCHIP8_HPP
