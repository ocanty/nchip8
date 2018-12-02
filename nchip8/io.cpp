//
// Created by ocanty on 10/10/18.
//

#include "io.hpp"

namespace nchip8
{

std::stringstream log;

std::ostream& nnn(std::ostream& out)
{
    return out << std::showbase << std::setfill('0') << std::setw(3) << std::hex;
}

std::ostream& n(std::ostream& out)
{
    return out << std::showbase << std::setfill('0') << std::setw(1) << std::hex;
}

std::ostream& V(std::ostream& out)
{
    return out << "V" << std::hex << std::uppercase;
}

std::ostream& kk(std::ostream& out)
{
    return out << std::showbase << std::setfill('0') << std::setw(2) << std::hex;
}

std::ostream& x(std::ostream& out)
{
    return out << std::setfill('0') << std::setw(2) << std::hex;
}

std::ostream& y(std::ostream& out)
{
    return out << std::setfill('0') << std::setw(2) << std::hex;
}

std::ostream& clr(std::ostream& out)
{
    std::cout.fill();
    return out << std::dec << std::setw(1);
}


}