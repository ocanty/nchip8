//
// Created by ocanty on 10/10/18.
//

#include "io.hpp"

namespace nchip8
{

std::stringstream log;

std::ostream& inst(std::ostream& out)
{
    return out << std::showbase << std::setfill('0') << std::setw(4) << std::hex;
}

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
    return out << "V" << std::noshowbase << std::hex << std::uppercase;
}

std::ostream& kk(std::ostream& out)
{
    return out << std::showbase << std::setfill('0') << std::setw(2) << std::hex;
}

}