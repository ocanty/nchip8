//
// Created by ocanty on 25/09/18.
//

#include "cpu.hpp"
#include <iostream>
#include <sstream>

namespace nchip8
{

cpu::cpu()
{
    this->setup_op_handlers();
    this->reset();
}

cpu::~cpu()
{

}

void cpu::reset()
{
    //std::cout << "[cpu] reset" << std::endl;

    m_gpr.fill(0);
    m_ram.fill(0xCC);

    m_pc = 0x200;
    m_stack.fill(0xBEEF); // fill the stack with junk

}

bool cpu::load_rom(const std::vector<std::uint8_t> &rom, const uint16_t load_addr)
{
    std::cout << "[cpu] loading rom" << std::endl;

    // Make sure the rom does not exceed typical loading size
    // Make sure it'l fit into the remainder of memory from it's load point
    if (rom.size() < 0xE00 && (load_addr + rom.size()) < 0x1000)
    {
        std::copy_n(rom.begin(), rom.size(), m_ram.begin() + load_addr);
        //std::cout << "[cpu] loading rom succeeded :)" << std::endl;
        return true;
    }

    std::cout << "[cpu] loading rom failed :(" << std::endl;
    return false;
}


std::optional<cpu::op_handler> cpu::get_op_handler_for_instruction(const std::uint16_t &instruction)
{
    // alias, not really relevant
    const std::uint16_t &op = instruction;

    // for an instruction 0xABCD
    // we get each nibble, so nibble0 = 0xA, nibble1 = 0xB
    std::uint8_t n3 = (op & 0x000F);
    std::uint8_t n2 = (op & 0x00F0) >> 4;
    std::uint8_t n1 = (op & 0x0F00) >> 8;
    std::uint8_t n0 = (op & 0xF000) >> 12;

    const auto &root = m_op_tree;

    if (root.count(n0) > 0)
    {
        auto &node0 = m_op_tree[n0];

        std::cout << node0.count(n1) << " " << node0.count(std::nullopt) << std::endl;

        // if we cant find a node that contains the next nibble
        // and cant find operaznd data (optional type), there is no handler, return nothing
        if (!node0.count(n1) && !node0.count(std::nullopt)) return std::nullopt;
        auto &node1 = (node0.count(n1) ? node0[n1] : node0[std::nullopt]);

        // repeat for next node
        if (!node1.count(n2) && !node1.count(std::nullopt)) return std::nullopt;
        auto &node2 = (node1.count(n2) ? node1[n2] : node1[std::nullopt]);

        // repeat for next node
        if (!node2.count(n3) && !node2.count(std::nullopt)) return std::nullopt;
        auto &node3 = (node2.count(n3) ? node2[n3] : node2[std::nullopt]);

        return node3; // the op_handler at the lowest leaf of the tree
    }

    // no handler found, invalid instruction :(
    return std::nullopt;
}

void cpu::execute_op_at_pc()
{
    std::uint16_t instruction = this->read_u16(this->m_pc);

    // get an operation handler for the instruction at PC
    std::optional<op_handler> handler = get_op_handler_for_instruction(instruction);

    // save the program counter, we will compare it after execution to see if a jump was performed
    std::uint16_t saved_pc = this->m_pc;

    if (handler != std::nullopt)
    {
        // now extract the vars from the instruction in order to supply to the handlers

        operand_data operands;
        operands.m_nnn = (instruction & 0x0FFF);
        operands.m_x = (instruction & 0x0F00) >> 8;
        operands.m_y = (instruction & 0x00F0) >> 4;
        operands.m_kk = (instruction & 0x00FF);
        operands.m_n = (instruction & 0x000F);

        std::cout << "got a handler" << std::endl;
        handler.value().m_execute_op(*this,operands);

        return;
    }

    //std::cout << "Illegal instruction :(" << std::endl;
}

std::optional<std::string> cpu::dasm_op(const std::uint16_t& address) const
{
    //const auto& handler = get_op_handler_for_pc();

    return "UNKWN";
}

std::uint16_t cpu::read_u16(const std::uint16_t &addr) const
{
    return (m_ram[addr] << 8 | m_ram[addr + 1]);
}

void cpu::set_u16(const std::uint16_t &addr, const std::uint16_t &val)
{
    m_ram[addr] = val >> 8;
    m_ram[addr + 1] = val & 0x00FF;
}

void cpu::add_op_handler(const cpu::op_handler &handler)
{
    auto &root = m_op_tree;

    if (!root.count(handler.m_encoding[0])) root[handler.m_encoding[0]] = {};
    auto &node0 = root[handler.m_encoding[0]];

    // if a child doesnt exist create it
    if (!node0.count(handler.m_encoding[1])) node0[handler.m_encoding[1]] = {};
    auto &node1 = node0[handler.m_encoding[1]];

    // if a child doesnt exist create it
    if (!node1.count(handler.m_encoding[2])) node1[handler.m_encoding[2]] = {};
    auto &node2 = node1[handler.m_encoding[2]];

    node2[handler.m_encoding[3]] = handler;
}

void cpu::setup_op_handlers()
{
    add_op_handler(cpu::JP);

//    // 0x00EE RET
//    add_op_handler(
//        op_handler
//        {
//            { 0x0, 0x0, 0xE, 0xE },
//            [this](const cpu::operand_data& operands)
//            {
//                m_pc = this->m_stack[m_sp];
//                this->m_sp--;
//            },
//
//            [](const cpu::operand_data& operands, std::stringstream& ss)
//            {
//                ss << "RET" << std::hex << operands.m_nnn;
//            }
//        }
//    );
//
//
//    // 0x1nnn - JP addr
//    // Jump to location nnn.
//    add_op_handler
//    (
//        op_handler
//        {
//            { 0x1, std::nullopt, std::nullopt, std::nullopt },
//            [this](const cpu::operand_data& operands)
//            {
//                // JP 0x1NNN
//                this->m_pc = operands.m_nnn;
//            },
//
//            [](const cpu::operand_data& operands, std::stringstream& ss)
//            {
//                ss << "JP 0x" << std::hex << operands.m_nnn;
//            }
//        }
//    );
//
//    // 0x2nnn - CALL addr
//    // Call subroutine at nnn.
//    add_op_handler(
//        op_handler
//        {
//            { 0x2, std::nullopt, std::nullopt, std::nullopt },
//            [this](const cpu::operand_data& operands)
//            {
//                this->m_sp++;
//                this->m_stack[m_sp] = m_pc;
//                m_pc = operands.m_nnn;
//            },
//
//            [](const cpu::operand_data& operands, std::stringstream& ss)
//            {
//                ss << "CALL 0x" << std::hex << operands.m_nnn;
//            }
//        }
//    );
//
//    // 0x3xkk - SE Vx, byte
//    // Skip next instruction if Vx = kk.
//    add_op_handler(
//        op_handler
//        {
//            { 0x3, std::nullopt, std::nullopt, std::nullopt },
//            [this](const cpu::operand_data& operands)
//            {
//                if(m_gpr[operands.m_x] == operands.m_kk) this->m_pc += 0x2;
//            },
//
//            [](const cpu::operand_data& operands, std::stringstream& ss)
//            {
//                ss << "SE V" << std::hex << operands.m_x << ", " << operands.m_kk;
//            }
//        }
//    );
//
//    // 0x4xkk - SNE Vx, byte
//    // Skip next instruction if Vx != kk.
//    add_op_handler(
//        op_handler
//        {
//            { 0x4, std::nullopt, std::nullopt, std::nullopt },
//            [this](const cpu::operand_data& operands)
//            {
//                if(m_gpr[operands.m_x] != operands.m_kk) this->m_pc += 0x2;
//            },
//
//            [](const cpu::operand_data& operands, std::stringstream& ss)
//            {
//                ss << "SNE V" << std::hex << operands.m_x << ", " << operands.m_kk;
//            }
//        }
//    );
//
//    // 0x5xy0 - SE Vx, Vy
//    // Skip next instruction if Vx == Vy.
//    add_op_handler(
//        op_handler
//        {
//            { 0x5, std::nullopt, std::nullopt, std::nullopt },
//            [this](const cpu::operand_data& operands)
//            {
//                if(m_gpr[operands.m_x] == m_gpr[operands.m_y]) this->m_pc += 0x2;
//            },
//
//            [](const cpu::operand_data& operands, std::stringstream& ss)
//            {
//                ss << "SE V" << std::hex << operands.m_x << ", V" << operands.m_y;
//            }
//        }
//    );
//
//    // 0x6xkk - LD Vx, byte
//    // Set Vx = kk.
//    add_op_handler(
//        op_handler
//        {
//            { 0x6, std::nullopt, std::nullopt, std::nullopt },
//            [this](const cpu::operand_data& operands)
//            {
//                m_gpr[operands.m_x] = operands.m_kk;
//            },
//
//            [](const cpu::operand_data& operands, std::stringstream& ss)
//            {
//                ss << "LD V" << std::hex << operands.m_x << ", " << operands.m_kk;
//            }
//        }
//    );
//
//    // 0x7xkk - ADD Vx, byte
//    // Set Vx = Vx + kk
//    add_op_handler(
//        op_handler
//        {
//            { 0x7, std::nullopt, std::nullopt, std::nullopt },
//            [this](const cpu::operand_data& operands)
//            {
//                m_gpr[operands.m_x] += operands.m_kk;
//            },
//
//            [](const cpu::operand_data& operands, std::stringstream& ss)
//            {
//                ss << "ADD V" << std::hex << operands.m_x << ", " << operands.m_kk;
//            }
//        }
//    );
//
//    // 0x8xy0 - LD Vx, Vy
//    // Set Vx = Vy.
//    add_op_handler(
//        op_handler
//        {
//            { 0x8, std::nullopt, std::nullopt, 0x0 },
//            [this](const cpu::operand_data& operands)
//            {
//                m_gpr[operands.m_x] = m_gpr[operands.m_y];
//            },
//
//            [](const cpu::operand_data& operands, std::stringstream& ss)
//            {
//                ss << "LD V" << std::hex << operands.m_x << ", V" << operands.m_y;
//            }
//        }
//    );
//
//    // 0x8xy1 - OR Vx, Vy
//    // Set Vx = Vx OR Vy.
//    add_op_handler(
//        op_handler
//        {
//            { 0x8, std::nullopt, std::nullopt, 0x1 },
//            [this](const cpu::operand_data& operands)
//            {
//                m_gpr[operands.m_x] = m_gpr[operands.m_x] | m_gpr[operands.m_y];
//            },
//
//            [](const cpu::operand_data& operands, std::stringstream& ss)
//            {
//                ss << "OR V" << std::hex << operands.m_x << ", V" << operands.m_y;
//            }
//        }
//    );
//
//    // 0x8xy2 - AND Vx, Vy
//    // Set Vx = Vx AND Vy.
//    add_op_handler(
//        op_handler
//        {
//            { 0x8, std::nullopt, std::nullopt, 0x2 },
//            [this](const cpu::operand_data& operands)
//            {
//                m_gpr[operands.m_x] = m_gpr[operands.m_x] & m_gpr[operands.m_y];
//            },
//
//            [](const cpu::operand_data& operands, std::stringstream& ss)
//            {
//                ss << "AND V" << std::hex << operands.m_x << ", V" << operands.m_y;
//            }
//        }
//    );
//
//    // 0x8xy3 - XOR Vx, Vy
//    // Set Vx = Vx XOR Vy.
//    add_op_handler(
//        op_handler
//        {
//            { 0x8, std::nullopt, std::nullopt, 0x3 },
//            [this](const cpu::operand_data& operands)
//            {
//                m_gpr[operands.m_x] = m_gpr[operands.m_x] ^ m_gpr[operands.m_y];
//            },
//
//            [](const cpu::operand_data& operands, std::stringstream& ss)
//            {
//                ss << "XOR V" << std::hex << operands.m_x << ", V" << operands.m_y;
//            }
//        }
//    );
//
//    // 0x8xy4 - ADD Vx, Vy
//    // Set Vx = Vx + Vy.
//    // If the result is greater than 8 bits (i.e., > 255,) VF (carry) is set to 1, otherwise 0.
//    add_op_handler(
//        op_handler
//        {
//            { 0x8, std::nullopt, std::nullopt, 0x3 },
//            [this](const cpu::operand_data& operands)
//            {
//                std::uint16_t result = m_gpr[operands.m_x] + m_gpr[operands.m_y];
//
//                m_gpr[0xF] = 0;
//                if(result > 255) m_gpr[0xF] = 1;
//
//                m_gpr[operands.m_x] = result & 0x00FF; // remove upper 8 bits
//            },
//
//            [](const cpu::operand_data& operands, std::stringstream& ss)
//            {
//                ss << "ADD V" << std::hex << operands.m_x << ", V" << operands.m_y;
//            }
//        }
//    );
}

}