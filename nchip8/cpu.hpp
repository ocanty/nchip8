//
// Created by ocanty on 25/09/18.
//

#ifndef CHIP8_NCURSES_CPU_HPP
#define CHIP8_NCURSES_CPU_HPP

#include <array>
#include <memory>
#include <functional>
#include <string>
#include <unordered_map>
#include <optional>
#include <vector>
#include <variant>


class cpu
{
public:
    cpu();

    ~cpu();

    //! @brief  Resets the CPU state, clears RAM, registers, the stack, screen etc...
    void reset();

    //! @brief              Loads a ROM into the CPU
    //! @param  rom         A vector of 8-bit ints of the raw rom data
    //! @param  address     The memory location to load the rom into
    //! @returns            true if loading was successful, false otherwise
    bool load_rom(const std::vector<std::uint8_t>& rom, const std::uint16_t address);

    //! @brief Executes the current instruction at PC, (PC may jump or increment afterwards)
    void execute_op_at_pc();

    //! @brief          Returns a disassembly of the instruction at PC
    //! @param address  The address of the instruction, must be correctly aligned
    //! @returns        Optional of string of disassembled instruction
    std::optional<std::string> dasm_op(const std::uint16_t& address);

private:
    std::array<std::uint8_t,0x1000> m_ram;
    std::array<std::uint8_t, 16> m_gpr;
    std::uint16_t m_i; // I register, only uses right-most 12 bits

    std::uint16_t m_pc;
    std::uint8_t m_sp;

    std::array<std::uint16_t, 16> m_stack;

    //! @brief          Reads a 16-bit value at the specified address
    //! @param address  The address
    std::uint16_t read_u16(const std::uint16_t& address);

    //! @brief          Sets a 16-bit value at the specified address
    //! @param address  The address
    //! @param val      The new value
    void set_u16(const std::uint16_t& address, const std::uint16_t& val);

    //! @brief      The operands that an instruction may have, passed to execute/dasm functions
    //! @details    CHIP-8 instruction breakdown:
    //!             Any CHIP-8 instruction can take the following forms:
    //!             0xANNN, 0xAKKA, 0xAXYA, 0xAAAN, 0xAXYN, 0xAAAA
    //!             where A is strictly defined (i.e specifies the instruction)
    //!             NNN, X, Y, KK, and N are considered operand values
    struct operand_data
    {
        std::uint16_t     m_nnn; // NNN
        std::uint8_t      m_x;   // X
        std::uint8_t      m_y;   // Y
        std::uint8_t      m_kk;  // KK
        std::uint8_t      m_n;   // N
    };

    //! @brief  A function type that when executed,
    //!         should process the instruction operation and update the relevant parts of the CPU
    using func_execute_op = std::function<void(const operand_data&)>;

    //! @brief  A function that when called,
    //!         returns the disassembly string of the instruction
    using func_dasm_op = std::function<void(const operand_data&)>;

    //! @brief  Container type to hold both functions that could process an instruction
    //!         both an execution and a disassembly routine
    struct op_handler
    {
        func_execute_op  m_execute_op;
        func_dasm_op        m_dasm_op;
    };

    //! @brief      4 nested maps, indexed by each nibble of the instruction
    //!
    //! @details    4bit nibbles in this case are using an 8bit type
    //!             Operand data is indexed as optional (std::nullopt)
    std::unordered_map<std::optional<std::uint8_t>,
            std::unordered_map<std::optional<std::uint8_t>,
                    std::unordered_map<std::optional<std::uint8_t>,
                            std::unordered_map<std::optional<std::uint8_t>,
                                    op_handler>>>> m_op_tree;

    //! @brief          Returns the operation handler for an instruction
    //! @param address  The encoded instruction (i.e 0X1200 - JP 200)
    //! @returns        Optional of operation handler if successful, std::nullopt if false
    std::optional<op_handler> get_op_handler_for_instruction(const std::uint16_t& instruction);

    //! @brief          Add an operation handler for an instruction into the handler tree
    //! @param encoding An array specifying the instruction encoding
    //!                 e.g. 0x1NNN - { 0x1, std::nullopt, std::nullopt, std::nullopt }
    //! @param handler  Handler structure, containing an execute and disassembly function
    void add_op_handler(
            const std::array<std::optional<std::uint8_t>, 4>& encoding,
            const op_handler& handler
        );

    //! @brief Add all the CHIP-8 operation handlers to the operation tree
    void setup_op_handlers();

};


#endif //CHIP8_NCURSES_CPU_HPP
