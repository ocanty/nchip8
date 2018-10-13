//
// Created by ocanty on 10/10/18.
//

#ifndef NCHIP8_LOG_HPP
#define NCHIP8_LOG_HPP

#include <iostream>
#include <sstream>
#include <iomanip>

namespace nchip8
{

//! Global log, variable exists in io.cpp.
//! @details NOT thread-safe!
//! @see io.cpp
//! @see gui.hpp, update_logs_on_global_log_change is what empties this log to gui
extern std::stringstream log;

std::ostream& nnn(std::ostream& out);
std::ostream& n(std::ostream& out);
std::ostream& kk(std::ostream& out);
std::ostream& x(std::ostream& out);
std::ostream& y(std::ostream& out);

}

#endif //NCHIP8_LOG_HPP
