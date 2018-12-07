//
// Created by ocanty on 09/10/18.
//

#ifndef NCHIP8_OP_HANDLERS_HPP
#define NCHIP8_OP_HANDLERS_HPP

#include <sstream>
#include <iostream>
#include <bitset>
#include <random>

#include "cpu.hpp"
#include "io.hpp"


// This file includes implementations of the static cpu:: op_handlers

namespace nchip8
{

//! The instruction encoding for these operations uses std::nullopt to
//! specify a field with operand data
//! for the purposes of space and readability, we are aliasing this to "operand"
static constexpr std::nullopt_t DATA = std::nullopt;

cpu::op_handler cpu::CLS
{
    {0x0, 0x0, 0xE, 0x0},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_screen.fill(0);
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "CLS";
    }
};


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
        ss << "RET ";
    }
};

cpu::op_handler cpu::JP
{
    {0x1, DATA, DATA, DATA},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
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
        cpu.m_sp++; // get space on the stack to store return value

        // store return address (which is the instruction after current PC)
        cpu.m_stack[cpu.m_sp] = cpu.m_pc + 0x2;

        // jump
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

        if(cpu.m_gpr[operands.m_x] == operands.m_kk) {
            cpu.m_pc += 0x4;
        }
    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "SE " << nchip8::V << operands.m_x << ", " <<  nchip8::kk << operands.m_kk;
    }
};

// 0x4xkk - SNE Vx, byte
// Skip next instruction if Vx != kk.
cpu::op_handler cpu::SNE_VX_KK
{
    { 0x4, DATA, DATA, DATA },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        if(cpu.m_gpr[operands.m_x] != operands.m_kk) cpu.m_pc += 0x4;
    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "SNE " << nchip8::V << operands.m_x << ", " << nchip8::kk << operands.m_kk;
    }
};

// 0x5xy0 - SE Vx, Vy
// Skip next instruction if Vx == Vy.
cpu::op_handler cpu::SE_VX_VY
{
    { 0x5, DATA, DATA, 0x0 },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        if(cpu.m_gpr[operands.m_x] == cpu.m_gpr[operands.m_y]) cpu.m_pc += 0x4;
    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "SE " << nchip8::V << operands.m_x << ", " << nchip8::V << operands.m_y;
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
        ss << "LD "  << nchip8::V << operands.m_x << ", " << nchip8::kk << operands.m_kk;

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
        ss << "ADD " << nchip8::V << operands.m_x << ", " << nchip8::kk << operands.m_kk;
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
        ss << "LD " << nchip8::V << operands.m_x << ", " << nchip8::V << operands.m_y;
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
        ss << "OR " << nchip8::V << operands.m_x << ", " << nchip8::V << operands.m_y;
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
        ss << "AND " << nchip8::V << operands.m_x << ", " << nchip8::V << operands.m_y;
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
        ss << "XOR " << nchip8::V << operands.m_x << ", " << nchip8::V << operands.m_y;
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
        ss << "ADD " << nchip8::V << operands.m_x << ", " << nchip8::V << operands.m_y;
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
        ss << "SUB " << nchip8::V << operands.m_x << ", " << nchip8::V << operands.m_y;
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
        ss << "SHR " << nchip8::V << operands.m_x; // << ", " << nchip8::V << operands.m_y;
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
        ss << "SUBN " << nchip8::V << operands.m_x  << ", " << nchip8::V << operands.m_y;
    }
};

// 0x8xyE - SHL Vx {,Vy }
// Set Vx = Vx SHR 1.
//
// Before this, if the most-significant bit of Vx (before shift) is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
cpu::op_handler cpu::SHL_VX_VY
{
    { 0x8, DATA, DATA, 0xE },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_gpr[0xF] = (cpu.m_gpr[operands.m_x] & 0x0001 ? 1 : 0); // LSB
        cpu.m_gpr[operands.m_x] <<= 1;

    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "SHL " << nchip8::V << operands.m_x; // << ", " << nchip8::V << operands.m_y;
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
        if(cpu.m_gpr[operands.m_x] != cpu.m_gpr[operands.m_y]) cpu.m_pc += 0x4;
        // skip current and go to 1 after
    },

    [](const cpu::operand_data& operands, std::stringstream& ss)
    {
        ss << "SNE " << nchip8::V << operands.m_x << ", " << nchip8::V << operands.m_y;
    }
};

// Annn - LD I, addr
// Set I = nnn.
cpu::op_handler cpu::LD_I_NNN
{
    {0xA, DATA, DATA, DATA},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_i = operands.m_nnn;
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "LD I, " << nchip8::nnn << operands.m_nnn;
    }
};

// Bnnn - JP V0, addr
// Jump to location nnn + V0.
cpu::op_handler cpu::JP_V0_NNN
{
    {0xB, DATA, DATA, DATA},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_pc = operands.m_nnn + cpu.m_gpr[0x0];
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "JP V0, " << nchip8::nnn << operands.m_nnn;
    }
};

// Cxkk - RND Vx, byte
// Set Vx = random byte AND kk.
cpu::op_handler cpu::RND_VX_KK
{
    {0xC, DATA, DATA, DATA},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        std::default_random_engine random(std::random_device{}());
        std::uniform_int_distribution<std::uint8_t> dist{ 0, 255 };

        cpu.m_gpr[operands.m_x] = (dist(random) & operands.m_kk);
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "RND " << nchip8::V << operands.m_x << ", " << nchip8::V << operands.m_y;
    }
};

// Dxyn - DRW Vx, Vy, nibble
// Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
cpu::op_handler cpu::DRW_VX_VY_N
{
    { 0xD, DATA, DATA, DATA },
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        int x = cpu.m_gpr[operands.m_x];
        int y = cpu.m_gpr[operands.m_y];

        for(int n = 0; n < operands.m_n; n++)
        {
            std::uint8_t line = cpu.m_ram.at(cpu.m_i + n);
            std::bitset<8> sprite_byte(line);

            for(int i = 0; i < 8 ; i++)
            {
                auto on = cpu.get_screen_xy(x, y);
                if(on == true && sprite_byte[7-i] == false) { cpu.m_gpr[0xF] = 1; }
                cpu.set_screen_xy(x, y, sprite_byte[7-i] ^ on);
                x += 1;

                x %= 64;
            }
            x = cpu.m_gpr[operands.m_x];
            y++;
            y %= 32;
        }
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "DRW " << nchip8::V << operands.m_x << ", " << nchip8::V << operands.m_y << ", " << nchip8::n << operands.m_n;
    }
};

// Ex9E - SKP Vx
// Skip next instruction if key with the value of Vx is pressed.
cpu::op_handler cpu::SKP_VX
{
    {0xE, DATA, 0x9, 0xE},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        if(cpu.m_key_down == cpu.m_gpr[operands.m_x])
        {
            cpu.m_pc += 0x4;
        }
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "SKP " << nchip8::V << operands.m_x;
    }
};

// ExA1 - SKNP Vx
// Skip next instruction if key with the value of Vx is not pressed.
cpu::op_handler cpu::SKNP_VX
{
    {0xE, DATA, 0xA, 0x1},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        if(cpu.m_key_down != cpu.m_gpr[operands.m_x])
        {
            cpu.m_pc += 0x4;
        }
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "SKNP " << nchip8::V << operands.m_x;
    }
};

// Fx07 - LD Vx, DT
// Set Vx = delay timer value.
cpu::op_handler cpu::LD_VX_DT
{
    {0xF, DATA, 0x0, 0x7},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_gpr[operands.m_x] = cpu.m_dt;
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "LD " << nchip8::V << operands.m_x << ", DT";
    }
};

// Fx0A - LD Vx, K
// Wait for a key press, store the value of the key in Vx.
cpu::op_handler cpu::LD_VX_K
{
    {0xF, DATA, 0x0, 0xA},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
         // wait for the value of key to change to valid
        while(cpu.m_key_down == std::nullopt)
        {

        }

        if(cpu.m_key_down != std::nullopt)
        {
            cpu.m_gpr[operands.m_x] = cpu.m_key_down.value();
        }
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "LD " << nchip8::V << operands.m_x << ", K";
    }
};

// Fx15 - LD DT, Vx
// Set delay timer = Vx.
cpu::op_handler cpu::LD_DT_VX
{
    {0xF, DATA, 0x1, 0x5},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_dt = cpu.m_gpr[operands.m_x];
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "LD DT, " << nchip8::V << operands.m_x;
    }
};

// Fx18 - LD ST, Vx
// Set sound timer = Vx.
cpu::op_handler cpu::LD_ST_VX
{
    {0xF, DATA, 0x1, 0x8},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_st = cpu.m_gpr[operands.m_x];
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "LD ST, " << nchip8::V << operands.m_x;
    }
};

// Fx1E - ADD I, Vx
// Set I = I + Vx.
//
// The values of I and Vx are added, and the results are stored in I.
cpu::op_handler cpu::ADD_I_VX
{
    {0xF, DATA, 0x1, 0xE},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        cpu.m_i += cpu.m_gpr[operands.m_x];
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "ADD I, " << nchip8::V << operands.m_x;
    }
};

//
// Fx29 - LD F, Vx
// Set I = location of sprite for digit Vx.
cpu::op_handler cpu::LD_F_VX
{
    {0xF, DATA, 0x2, 0x9},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        // previously we copied the font to the base of memory
        // each fonts char data has a width of 5 bytes, and is stored sequentially
        cpu.m_i = cpu.m_gpr[operands.m_x]*0x5;
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "LD F, " << nchip8::V << operands.m_x;
    }
};

// Fx33 - LD B, Vx
cpu::op_handler cpu::LD_B_VX
{
    {0xF, DATA, 0x3, 0x3},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        // TODO: implement
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "LD F, " << nchip8::V << operands.m_x;
    }
};

//Fx55 - LD [I], Vx
//Store registers V0 through Vx in memory starting at location I.
//
//The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.
cpu::op_handler cpu::LD_imm_I_VX
{
    {0xF, DATA, 0x5, 0x5},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        for(int i = 0; i < operands.m_x; i++)
        {
            cpu.m_ram[cpu.m_i + i] = cpu.m_gpr[i];
        }

        cpu.m_i = cpu.m_i = operands.m_x + 1;
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "LD [I], " << nchip8::V << operands.m_x;
    }
};

//Fx65 - LD Vx, [I]
//Read registers V0 through Vx from memory starting at location I.
//
//The interpreter reads values from memory starting at location I into registers V0 through Vx.
cpu::op_handler cpu::LD_VX_imm_I
{
    {0xF, DATA, 0x6, 0x5},
    [](cpu &cpu, const cpu::operand_data &operands)
    {
        for(int i = 0; i < operands.m_x; i++)
        {
            cpu.m_gpr[i] = cpu.m_ram[cpu.m_i + i];
        }

        cpu.m_i = cpu.m_i = operands.m_x + 1;
    },

    [](const cpu::operand_data &operands, std::stringstream &ss)
    {
        ss << "LD " << nchip8::V << operands.m_x << ", [I]";
    }
};
}
#endif //NCHIP8_OP_HANDLERS_HPP
