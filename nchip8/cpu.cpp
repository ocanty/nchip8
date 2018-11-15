//
// Created by ocanty on 25/09/18.
//

#include "cpu.hpp"
#include "io.hpp"
#include <iostream>
#include <sstream>
#include <tuple>
#include <utility>

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
    // clear registers and memory
    m_gpr.fill(0);
    m_ram.fill(0xCC);

    m_pc = 0x200;
    m_stack.fill(0xBEEF); // fill the stack with junk
}

bool cpu::load_rom(const std::vector<std::uint8_t> &rom, const uint16_t& load_addr)
{
    // Make sure the rom does not exceed typical loading size
    // Make sure it'l fit into the remainder of memory from it's load point
    if (rom.size() < 0xE00 && (load_addr + rom.size()) < 0x1000)
    {
        std::copy_n(rom.begin(), rom.size(), m_ram.begin() + load_addr);
        return true;
    }

    return false;
}

bool cpu::add_op_handler(const cpu::op_handler &handler)
{
    auto& root = m_op_tree;

    // add a node to the tree if we don't have one
    // see: https://en.cppreference.com/w/cpp/container/unordered_map/try_emplace
    // try_emplace supports perfect forwarding and inserting the nested map ( {} )
    // doesn't need args for it's constructor, we don't supply them
    auto [node_0_iter, node_0_success] = root.try_emplace(handler.m_encoding[0]);
    auto [node_1_iter, node_1_success] = node_0_iter->second.try_emplace(handler.m_encoding[1]);
    auto [node_2_iter, node_2_success] = node_1_iter->second.try_emplace(handler.m_encoding[2]);

    auto [iter, success] = node_2_iter->second.try_emplace(handler.m_encoding[3], handler);

    return success;
}

void cpu::setup_op_handlers()
{
    add_op_handler(cpu::RET);
    add_op_handler(cpu::JP);
    add_op_handler(cpu::CALL);
    add_op_handler(cpu::SE_VX_KK);
    add_op_handler(cpu::SNE_VX_KK);
    add_op_handler(cpu::SE_VX_VY);
    add_op_handler(cpu::LD_VX_KK);
    add_op_handler(cpu::ADD_VX_KK);
    add_op_handler(cpu::LD_VX_VY);
    add_op_handler(cpu::OR_VX_VY);
    add_op_handler(cpu::AND_VX_VY);
    add_op_handler(cpu::XOR_VX_VY);
    add_op_handler(cpu::ADD_VX_VY);
    add_op_handler(cpu::SUB_VX_VY);
    add_op_handler(cpu::SHR_VX_VY);
    add_op_handler(cpu::SUBN_VX_VY);
    add_op_handler(cpu::SHL_VX_VY);
    add_op_handler(cpu::SNE_VX_VY);
    add_op_handler(cpu::LD_I_NNN);
    add_op_handler(cpu::JP_V0_NNN);
    add_op_handler(cpu::RND_VX_KK);
}

std::optional<cpu::op_handler> cpu::get_op_handler_for_instruction(const std::uint16_t& instruction) const
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
        auto &node0 = m_op_tree.at(n0);

        // if we cant find a node that contains the next nibble
        // and cant find operand data (optional type), there is no handler, return nothing
        if (!node0.count(n1) && !node0.count(std::nullopt)) return std::nullopt;

        // our next node is either indexed by an instruction nibble or an operand data (optional type)
        auto &node1 = (node0.count(n1) ? node0.at(n1) : node0.at(std::nullopt));

        // repeat for next node
        if (!node1.count(n2) && !node1.count(std::nullopt)) return std::nullopt;
        auto &node2 = (node1.count(n2) ? node1.at(n2) : node1.at(std::nullopt));

        // repeat for next node
        if (!node2.count(n3) && !node2.count(std::nullopt)) return std::nullopt;
        auto &node3 = (node2.count(n3) ? node2.at(n3) : node2.at(std::nullopt));

        return node3; // the op_handler at the lowest leaf of the tree
    }

    // no handler found, invalid instruction :(
    return std::nullopt;
}

cpu::operand_data cpu::get_operand_data_from_instruction(const std::uint16_t& instruction) const
{
    // extract operand data from 0xABCD
    operand_data operands;
    operands.m_nnn  = (instruction & 0x0FFF);       // 0xANNN
    operands.m_x    = (instruction & 0x0F00) >> 8;  // 0xAXCD
    operands.m_y    = (instruction & 0x00F0) >> 4;  // 0xABYD
    operands.m_kk   = (instruction & 0x00FF);       // 0xABKK
    operands.m_n    = (instruction & 0x000F);       // 0xABCN
    return operands;
}

void cpu::execute_op_at_pc()
{
    // read the encoded instruction
    std::uint16_t instruction = this->read_u16(this->m_pc);

    // get an operation handler for the instruction at PC
    std::optional<op_handler> handler = get_op_handler_for_instruction(instruction);

    // save the program counter, we will compare it after execution to see if a jump was performed
    std::uint16_t saved_pc = this->m_pc;

    if (handler != std::nullopt)
    {
        // now extract the vars from the instruction in order to supply to the handlers

        operand_data operands = get_operand_data_from_instruction(instruction);

        handler.value().m_execute_op(*this,operands);

        handler.value().m_dasm_op(operands,nchip8::log);

        // print out each register
        for(std::uint16_t r = 0; r < 16; r++)
        {
            nchip8::log << " V" << std::hex << r << " " << (std::uint16_t)this->m_gpr[r];
        }

        nchip8::log << std::endl;

        this->set_screen_xy(2,2,true);

        if(saved_pc == this->m_pc) // if pc wasnt modified by the executing function
        {
            // go to the next instruction
            this->m_pc+=2;
        }

        return;
    }
}

std::optional<std::string> cpu::dasm_op(const std::uint16_t& address) const
{
    std::uint16_t instruction = this->read_u16(address);

    // get an operation handler for the instruction at PC
    std::optional<op_handler> handler = get_op_handler_for_instruction(instruction);

    if (handler != std::nullopt)
    {
        // now extract the vars from the instruction in order to supply to the handlers
        operand_data operands = get_operand_data_from_instruction(instruction);

        static std::stringstream dasm;
        dasm.clear();

        handler.value().m_dasm_op(operands, dasm);
        dasm << std::endl;

        return dasm.str();
    }
    return std::nullopt;
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

const cpu::screen_mode &cpu::get_screen_mode() const
{
    return m_screen_mode;
}

const std::array<bool, 128*64>& cpu::get_screen_framebuffer() const
{
    return m_screen;
}

void cpu::set_screen_mode(const cpu::screen_mode &mode)
{
    m_screen_mode = mode;
}

bool cpu::get_screen_xy(const std::uint8_t &x, const std::uint8_t &y) const
{
     // https://stackoverflow.com/a/2151141 : width * row + col
    auto width = (get_screen_mode() == screen_mode::hires_sc8) ? 128 : 64;

    return m_screen[width*y+x];
}

void cpu::set_screen_xy(const std::uint8_t &x, const std::uint8_t &y, const bool &set)
{
    // https://stackoverflow.com/a/2151141 : width * row + col
    auto width = (get_screen_mode() == screen_mode::hires_sc8) ? 128 : 64;

    m_screen[width*y+x] = set;
}

}