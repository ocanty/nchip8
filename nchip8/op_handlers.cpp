//
// Created by ocanty on 09/10/18.
//

#ifndef NCHIP8_OP_HANDLERS_HPP
#define NCHIP8_OP_HANDLERS_HPP

#include <sstream>
#include <iostream>
#include <bitset>
#include "cpu.hpp"
#include "io.hpp"


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
        ss << "JP " << nchip8::nnn << operands.m_nnn;
    }
};

cpu::op_handler cpu::CALL
{
    { 0x2, DATA, DATA, DATA },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_sp++;
        cpu.m_stack[cpu.m_sp] = cpu.m_pc;
        cpu.m_pc = operands.m_nnn;
    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "CALL " << nchip8::nnn << operands.m_nnn;
    }
};

// 0x3xkk - SE Vx, byte
// Skip next instruction if Vx = kk.
cpu::op_handler cpu::SE_VX_KK
{
    { 0x3, DATA, DATA, DATA },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        if(cpu.m_gpr[operands.m_x] == operands.m_kk) cpu.m_pc += 0x2;
    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "SE V" << nchip8::x << operands.m_x << ", " <<  nchip8::kk << operands.m_kk;
    }
};

// 0x4xkk - SNE Vx, byte
// Skip next instruction if Vx != kk.
cpu::op_handler cpu::SNE_VX_KK
{
    { 0x4, DATA, DATA, DATA },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        if(cpu.m_gpr[operands.m_x] != operands.m_kk) cpu.m_pc += 0x2;
    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "SNE V" << nchip8::x << operands.m_x << ", " << nchip8::kk << operands.m_kk;
    }
};

// 0x5xy0 - SE Vx, Vy
// Skip next instruction if Vx == Vy.
cpu::op_handler cpu::SE_VX_VY
{
    { 0x5, DATA, DATA, 0x0 },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        if(cpu.m_gpr[operands.m_x] == cpu.m_gpr[operands.m_y]) cpu.m_pc += 0x2;
    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "SE V" << nchip8::x << operands.m_x << ", V" << nchip8::y << operands.m_y;
    }
};

// 0x6xkk - LD Vx, byte
// Set Vx = kk.
cpu::op_handler cpu::LD_VX_KK
{
    { 0x6, DATA, DATA, DATA },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_gpr[operands.m_x] = operands.m_kk;
    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "LD V" << nchip8::x << operands.m_x << ", " << nchip8::kk << operands.m_kk;
    }
};

// 0x7xkk - ADD Vx, byte
// Set Vx = Vx + kk
cpu::op_handler cpu::ADD_VX_KK
{
    { 0x7, DATA, DATA, DATA },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_gpr[operands.m_x] += operands.m_kk;
    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "ADD V" << nchip8::x << operands.m_x << ", " << nchip8::kk << operands.m_kk;
    }
};

// 0x8xy0 - LD Vx, Vy
// Set Vx = Vy.
cpu::op_handler cpu::LD_VX_VY
{
    { 0x8, DATA, DATA, 0x0 },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_gpr[operands.m_x] = cpu.m_gpr[operands.m_y];
    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "LD V" << nchip8::x << operands.m_x << ", V" << nchip8::y << operands.m_y;
    }
};

// 0x8xy1 - OR Vx, Vy
// Set Vx = Vx OR Vy.
cpu::op_handler cpu::OR_VX_VY
{
    { 0x8, DATA, DATA, 0x1 },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_gpr[operands.m_x] = cpu.m_gpr[operands.m_x] | cpu.m_gpr[operands.m_y];
    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "OR V" << nchip8::x << operands.m_x << ", V" << nchip8::y << operands.m_y;
    }
};

// 0x8xy2 - AND Vx, Vy
// Set Vx = Vx AND Vy.
cpu::op_handler cpu::AND_VX_VY
{
    { 0x8, DATA, DATA, 0x2 },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_gpr[operands.m_x] = cpu.m_gpr[operands.m_x] & cpu.m_gpr[operands.m_y];
    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "AND V" << nchip8::x << operands.m_x << ", V" << nchip8::y << operands.m_y;
    }
};

// 0x8xy3 - XOR Vx, Vy
// Set Vx = Vx XOR Vy.
cpu::op_handler cpu::XOR_VX_VY
{
    { 0x8, DATA, DATA, 0x3 },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_gpr[operands.m_x] = cpu.m_gpr[operands.m_x] ^ cpu.m_gpr[operands.m_y];
    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "XOR V" << nchip8::x << operands.m_x << ", V" << nchip8::y << operands.m_y;
    }
};

// 0x8xy4 - ADD Vx, Vy
// Set Vx = Vx + Vy.
// If the result is greater than 8 bits (i.e., > 255,) VF (carry) is set to 1, other
cpu::op_handler cpu::ADD_VX_VY
{
    { 0x8, DATA, DATA, 0x4 },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        std::uint16_t result = cpu.m_gpr[operands.m_x] + cpu.m_gpr[operands.m_y];

        cpu.m_gpr[0xF] = 0;
        if(result > 255) cpu.m_gpr[0xF] = 1;

        cpu.m_gpr[operands.m_x] = result & 0x00FF; // remove upper 8 bits
    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "ADD V" << nchip8::x << operands.m_x << ", V" << nchip8::y << operands.m_y;
    }
};

// 0x8xy5 - SUB Vx, Vy
// Set Vx = Vx - Vy, set VF = NOT borrow.
//
// If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
cpu::op_handler cpu::SUB_VX_VY
{
    { 0x8, DATA, DATA, 0x5 },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_gpr[0xF] = 0;
        if(cpu.m_gpr[operands.m_x] > cpu.m_gpr[operands.m_y]) cpu.m_gpr[0xF] = 1;

        cpu.m_gpr[operands.m_x] = cpu.m_gpr[operands.m_x] - cpu.m_gpr[operands.m_y];
    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "SUB V" << nchip8::x << operands.m_x << ", V" << nchip8::y << operands.m_y;
    }
};

// 0x8xy6 - SHR Vx,Vy
// Set Vx = Vx SHR 1.
//
// Before this, if the least-significant bit of Vx (before shift) is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
cpu::op_handler cpu::SHR_VX_VY
{
    { 0x8, DATA, DATA, 0x6 },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_gpr[0xF] = (cpu.m_gpr[operands.m_x] & 0x1 ? 1 : 0); // LSB
        cpu.m_gpr[operands.m_x] >>= 1;

    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "SHR " << nchip8::x << operands.m_x; // << ", V" << nchip8::y << operands.m_y;
    }
};

// 8xy7 - SUBN Vx, Vy
// Set Vx = Vy - Vx, set VF = NOT borrow.
//
// If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
cpu::op_handler cpu::SUBN_VX_VY
{
    { 0x8, DATA, DATA, 0x7 },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_gpr[0xF] = 0;
        if(cpu.m_gpr[operands.m_y] > cpu.m_gpr[operands.m_x]) cpu.m_gpr[0xF] = 1;

        cpu.m_gpr[operands.m_x] = cpu.m_gpr[operands.m_y] - cpu.m_gpr[operands.m_x];

    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "SUBN " << nchip8::x << operands.m_x  << ", V" << nchip8::y << operands.m_y;
    }
};

// 0x8xyE - SHL Vx {,Vy }
// Set Vx = Vx SHR 1.
//
// Before this, if the most-significant bit of Vx (before shift) is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
cpu::op_handler cpu::SHL_VX_VY
{
    { 0x8, DATA, DATA, 0x6 },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_gpr[0xF] = (cpu.m_gpr[operands.m_x] & 0x0001 ? 1 : 0); // LSB
        cpu.m_gpr[operands.m_x] <<= 1;

    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "SHL " << nchip8::x << operands.m_x; // << ", V" << nchip8::y << operands.m_y;
    }
};

// 9xy0 - SNE Vx, Vy
// Skip next instruction if Vx != Vy.
//
// The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
cpu::op_handler cpu::SNE_VX_VY
{
    { 0x9, DATA, DATA, 0x0 },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        if(cpu.m_gpr[operands.m_x] != cpu.m_gpr[operands.m_y]) cpu.m_pc += 0x2;
    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "SNE V" << nchip8::x << operands.m_x << ", V" << nchip8::y << operands.m_y;
    }
};

// Annn - LD I, addr
// Set I = nnn.
cpu::op_handler cpu::LD_I_NNN
{
    {0xA, DATA, DATA, DATA},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        // JP 0x1NNN
        cpu.m_i = operands.m_nnn;
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "LD I, " << nchip8::nnn << operands.m_nnn;
    }
};

// Bnnn - JP V0, addr
// Jump to location nnn + V0.
cpu::op_handler cpu::LD_I_NNN
{
    {0xB, DATA, DATA, DATA},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        // JP 0x1NNN
        cpu.m_pc = operands.m_nnn + cpu.m_gpr[0x0];
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "JP V0, " << nchip8::nnn << operands.m_nnn;
    }
};


}
#endif //NCHIP8_OP_HANDLERS_HPP
