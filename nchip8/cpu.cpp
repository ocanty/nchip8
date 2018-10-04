//
// Created by ocanty on 25/09/18.
//

#include "cpu.hpp"
#include <iostream>
#include <sstream>

cpu::cpu()
{

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
    //std::cout << "[cpu] loading rom" << std::endl;

    // Make sure the rom does not exceed typical loading size
    // Make sure it'l fit into the remainder of memory from it's load point
    if(rom.size() < 0xE00 && (load_addr + rom.size()) < 0x1000)
    {
        std::copy_n(rom.begin(), rom.size(), m_ram.begin()+load_addr);
        //std::cout << "[cpu] loading rom succeeded :)" << std::endl;
        return true;
    }

    //std::cout << "[cpu] loading rom failed :(" << std::endl;
    return false;
}



std::optional<cpu::op_handler> cpu::get_op_handler_for_instruction(const std::uint16_t& instruction)
{
    // alias, not really relevant
    const std::uint16_t& op = instruction;

    //std::cout << "Read operation -> " << std::hex << op << std::endl;

    // for an instruction 0xABCD
    // we get each nibble, so nibble0 = 0xA, nibble1 = 0xB, etc...
    std::uint8_t n3 = (op & 0x000F);
    std::uint8_t n2 = (op & 0x00F0) >> 4;
    std::uint8_t n1 = (op & 0x0F00) >> 8;
    std::uint8_t n0 = (op & 0xF000) >> 12;

    //std::cout << "PC: 0x" << std::hex << m_pc << " " << std::hex << nibble0 << nibble1 << nibble2 << nibble3 << std::endl;
    // check each byte in the op_handler tree

    const auto& root = m_op_tree;

    if(root.count(n0) > 0)
    {
        auto& node0 = m_op_tree[n0];

        // if we cant find a node that contains the next nibble or operand data (optional type), die
        if(!node0.count(n1) || !node0.count(std::nullopt)) return std::nullopt;
        auto& node1 = (node0.count(n1) ? node0[n1] : node0[std::nullopt]);

        // if we cant find a node that contains the next nibble or operand data (optional type), die
        if(!node1.count(n2) || !node1.count(std::nullopt)) return std::nullopt;
        auto& node2 = (node1.count(n2) ? node1[n2] : node1[std::nullopt]);

        // if we cant find a node that contains the next nibble or operand data (optional type), die
        if(!node2.count(n3) || !node2.count(std::nullopt)) return std::nullopt;
        auto& node3 = (node2.count(n3) ? node2[n3] : node2[std::nullopt]);

        return node3; // the op_handler at the base of the tree
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

    if(true/*handler != std::nullopt*/)
    {
        // now extract the vars from the instruction in order to supply to the handlers

        operand_data operands;
        operands.m_nnn = (instruction & 0x0FFF);
        operands.m_x   = (instruction & 0x0F00) >> 8;
        operands.m_y   = (instruction & 0x00F0) >> 4;
        operands.m_kk  = (instruction & 0x00FF);
        operands.m_n   = (instruction & 0x000F);

//        printf("nnn: %04x, x:%01x, y:%01x, kk:%01x, n:%01x ",
//                operands.m_nnn,
//                operands.m_x,
//                operands.m_y,
//                operands.m_kk,
//                operands.m_n);

        return;
    }

    //std::cout << "Illegal instruction :(" << std::endl;
}

std::optional<std::string> cpu::dasm_op(const std::uint16_t& address)
{
    //const auto& handler = get_op_handler_for_pc();

    return "UNKWN";
}

std::uint16_t cpu::read_u16(const std::uint16_t& addr)
{
    return (m_ram[addr] << 8 | m_ram[addr+1]);
}

void cpu::set_u16(const std::uint16_t &addr, const std::uint16_t &val)
{
    m_ram[addr] = val >> 8;
    m_ram[addr+1] = val & 0x00FF;
}

void cpu::add_op_handler(const std::array<std::optional<std::uint8_t>, 4>& encoding, const cpu::op_handler &handler)
{
    auto& root = m_op_tree;

    if(!root.count(encoding[0])) root[encoding[0]] = { };
    auto& node0 = root[encoding[0]];

    // if a child doesnt exist create it
    if(!node0.count(encoding[1])) node0[encoding[1]] = { };
    auto& node1 = node0[encoding[0]];

    // if a child doesnt exist create it
    if(!node1.count(encoding[2])) node1[encoding[2]] = { };
    auto& node2 = node1[encoding[1]];

    node2[encoding[3]] = op_handler{ };
}

void cpu::setup_op_handlers()
{
    this->add_op_handler(
        { 0x1, std::nullopt, std::nullopt, std::nullopt }, // 0x1NNN
        op_handler
        {
            [this](const cpu::operand_data& operands)
            {
                // JP 0x1NNN
                this->m_pc = operands.m_nnn;
            },

            [](const cpu::operand_data& operands) -> std::string
            {
                std::stringstream ss;
                ss << "JP 0x" << std::hex << operands.m_nnn;
                return ss.str();
            }
        }
    );
}
