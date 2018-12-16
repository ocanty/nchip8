//
// Created by ocanty on 25/09/18.
//

#include "cpu.hpp"
#include "io.hpp"
#include <iostream>
#include <sstream>
#include <tuple>
#include <utility>
#include <thread>
#include <chrono>
#include <algorithm>

namespace nchip8
{

cpu::cpu()
{
    this->setup_op_handlers();
    this->reset();
}

const std::vector<std::array<std::uint8_t,5>> font = {
    {0xF0, 0x90, 0x90, 0x90, 0xF0}, // 0
    {0x20, 0x60, 0x20, 0x20, 0x70}, // 1
    {0xF0, 0x10, 0xF0, 0x80, 0xF0}, // 2
    {0xF0, 0x10, 0xF0, 0x10, 0xF0}, // 3
    {0x90, 0x90, 0xF0, 0x10, 0x10}, // 4
    {0xF0, 0x80, 0xF0, 0x10, 0xF0}, // 5
    {0xF0, 0x80, 0xF0, 0x90, 0xF0}, // 6
    {0xF0, 0x10, 0x20, 0x40, 0x40}, // 7
    {0xF0, 0x90, 0xF0, 0x90, 0xF0}, // 8
    {0xF0, 0x90, 0xF0, 0x10, 0xF0}, // 9
    {0xF0, 0x90, 0xF0, 0x90, 0x90}, // A
    {0xE0, 0x90, 0xE0, 0x90, 0xE0}, // B
    {0xF0, 0x80, 0x80, 0x80, 0xF0}, // C
    {0xE0, 0x90, 0x90, 0x90, 0xE0}, // D
    {0xF0, 0x80, 0xF0, 0x80, 0xF0}, // E
    {0xF0, 0x80, 0xF0, 0x80, 0x80}  // F
};

void cpu::reset()
{
    // clear registers and memory
    m_gpr.fill(0);
    m_ram.fill(0x00);

    m_pc = 0x200;
    m_stack.fill(0x0000); // fill the stack with junk

    m_dt = 0;
    m_st = 0;

    // copy each byte of the font sprite into memory
    int i = 0;
    for(auto& character : font) {
        for(auto& byte : character) {
            m_ram[i] = byte;
            i++;
        }
    }

    m_keys_down.fill(false);
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
    add_op_handler(CLS);
    add_op_handler(RET);
    // add_op_handler(SYS);
    add_op_handler(JP);
    add_op_handler(CALL);
    add_op_handler(SE_VX_KK);
    add_op_handler(SNE_VX_KK);
    add_op_handler(SE_VX_VY);
    add_op_handler(LD_VX_KK);
    add_op_handler(ADD_VX_KK);
    add_op_handler(LD_VX_VY);
    add_op_handler(OR_VX_VY);
    add_op_handler(AND_VX_VY);
    add_op_handler(XOR_VX_VY);
    add_op_handler(ADD_VX_VY);
    add_op_handler(SUB_VX_VY);
    add_op_handler(SHR_VX_VY);
    add_op_handler(SUBN_VX_VY);
    add_op_handler(SHL_VX_VY);
    add_op_handler(SNE_VX_VY);
    add_op_handler(LD_I_NNN);
    add_op_handler(JP_V0_NNN);
    add_op_handler(RND_VX_KK);
    add_op_handler(DRW_VX_VY_N);
    add_op_handler(SKP_VX);
    add_op_handler(SKNP_VX);
    add_op_handler(LD_VX_DT);
    add_op_handler(LD_VX_K);
    add_op_handler(LD_DT_VX);
    add_op_handler(LD_ST_VX);
    add_op_handler(ADD_I_VX);
    add_op_handler(LD_F_VX);
    add_op_handler(LD_B_VX);
    add_op_handler(LD_imm_I_VX);
    add_op_handler(LD_VX_imm_I);
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
    static auto last_clock = std::chrono::high_resolution_clock::now();
    static bool kill_next_execute = false;

    // used to end execution if an error occurs
    if(kill_next_execute) return;

    // read the encoded instruction
    std::uint16_t instruction = this->read_u16(this->m_pc);

    // get an operation handler for the instruction at PC
    std::optional<op_handler> handler = get_op_handler_for_instruction(instruction);

    // if its a valid operation
    if (handler != std::nullopt)
    {
        // update the delay timer and sleep timer while we're at it
        // let's check how much time has passed since the last cpu execution
        auto delta_duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - last_clock
        ).count();

        // if more than a 60th of a second has passed
        if(delta_duration_ms > (1000/60)) {

            // discover the number of ticks that have passed, aka how many 60ths of a second have passed
            unsigned long ticks = delta_duration_ms / (1000/60);

            // update the clock
            last_clock = std::chrono::high_resolution_clock::now();

            if(ticks >= m_dt) { m_dt = 0; } else { m_dt -= ticks; }
            if(ticks >= m_st) { m_st = 0; } else { m_st -= ticks; }

        }

        // if the sound timer is non-zero sound a buzz
        if(m_st > 0) {
            // TODO: sound buzz on non-zero sound timer
        }

        // save the program counter,
        // we will compare it after execution to see if a jump was performed
        std::uint16_t saved_pc = this->m_pc;

        // now extract the vars from the instruction in order to supply to the handlers
        operand_data operands = get_operand_data_from_instruction(instruction);

        // disassemble and print to log
        nchip8::log << nchip8::nnn << this->m_pc << ' ';
        nchip8::log << " " << nchip8::inst << instruction << " ";
        handler.value().m_dasm_op(operands,nchip8::log);
        nchip8::log << std::endl;

        // execute the operation
        handler.value().m_execute_op(*this,operands);

        // if pc wasnt modified by the operation
        if(saved_pc == this->m_pc)
        {
            // go to the next instruction
            this->m_pc+=2;
        }

        return;
    }
    else {
        nchip8::log << "unhandled instruction: " << std::hex << instruction << std::endl;
        kill_next_execute = true;
    }
}

std::optional<std::string> cpu::dasm_op(const std::uint16_t& address) const
{
//    std::uint16_t instruction = this->read_u16(address);
//
//    // get an operation handler for the instruction at PC
//    std::optional<op_handler> handler = get_op_handler_for_instruction(instruction);
//
//    if (handler != std::nullopt)
//    {
//        // now extract the vars from the instruction in order to supply to the handlers
//        operand_data operands = get_operand_data_from_instruction(instruction);
//
//        static std::stringstream dasm;
//        dasm.clear();
//
//        handler.value().m_dasm_op(operands, dasm);
//        dasm << '\'
//
//        return dasm.str();
//    }
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

void cpu::set_key_down(const std::uint8_t &key)
{
    m_keys_down.at(key) = true;
    m_last_key_down = key;
}

void cpu::set_key_up(const std::uint8_t &key)
{
    m_keys_down.at(key) = false;

    if(key == m_last_key_down) {
        m_last_key_down = std::nullopt;
    }
}

}