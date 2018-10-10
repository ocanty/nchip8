//
// Created by ocanty on 09/10/18.
//

#ifndef NCHIP8_OP_HANDLERS_HPP
#define NCHIP8_OP_HANDLERS_HPP

#include <sstream>
#include <iostream>
#include "cpu.hpp"

// This file includes implementations of the static cpu:: op_handlers

namespace nchip8
{

//! The instruction encoding for these operations uses std::nullopt to
//! specify a field with operand data
//! for the purposes of space and readability, we are aliasing this to "operand"
static constexpr std::nullopt_t DATA = std::nullopt;

cpu::op_handler cpu::RET
{
    {0x0, 0x0, 0xE, 0xE},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_pc = cpu.m_stack[cpu.m_sp];
        cpu.m_sp--;
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "RET" << std::hex << operands.m_nnn;
    }
};

cpu::op_handler cpu::JP
{
    {0x1, DATA, DATA, DATA},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        // JP 0x1NNN
        std::cout << "hello!" << std::endl;
        cpu.m_pc = operands.m_nnn;
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "JP 0x" << std::hex << operands.m_nnn;
    }
};

}
#endif //NCHIP8_OP_HANDLERS_HPP
