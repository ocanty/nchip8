//
// Created by ocanty on 25/09/18.
//

#include "cpu.hpp"
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
    m_gpr.fill(0);
    m_ram.fill(0xCC);

    m_pc = 0x200;
    m_stack.fill(0xBEEF); // fill the stack with junk

}

bool cpu::load_rom(const std::vector<std::uint8_t> &rom, const uint16_t load_addr)
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

        // if we cant find a node that contains the next nibble
        // and cant find operand data (optional type), there is no handler, return nothing
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
        operands.m_nnn  = (instruction & 0x0FFF);
        operands.m_x    = (instruction & 0x0F00) >> 8;
        operands.m_y    = (instruction & 0x00F0) >> 4;
        operands.m_kk   = (instruction & 0x00FF);
        operands.m_n    = (instruction & 0x000F);

        handler.value().m_execute_op(*this,operands);

        return;
    }
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
    add_op_handler(cpu::JP);
    add_op_handler(cpu::CALL);
}

}