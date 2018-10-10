//
// Created by ocanty on 17/09/18.
//

#ifndef CHIP8_NCURSES_CPU_DAEMON_HPP
#define CHIP8_NCURSES_CPU_DAEMON_HPP


#include <thread>
#include <vector>
#include <mutex>
#include <functional>
#include <queue>
#include <functional>

#include "cpu.hpp"

namespace nchip8
{

//! @brief  The cpu_daemon creates the cpu thread,
//!         passes messages to the cpu and controls it's state
//!         e.g. calling for an instruction to be executed or not
class cpu_daemon
{
public:
    cpu_daemon();

    ~cpu_daemon();

    //! @brief Message type of message to be passed to cpu
    enum cpu_message_type : std::uint8_t
    {
        Reset,              //! Resets the cpu. Clear registers & ram, PC = 0x200   m_data: none
        LoadROM,            //! Writes a rom to cpu memory.                         m_data: vector of ROM binary
        SetStateRunning,    //! Set cpu running
        SetStatePaused,     //! Pause cpu at current instruction
        _last               // Used to find amount of messages, keep at end of enum
    };

    //! @brief Message container
    struct cpu_message
    {
        //! See cpu_message_type
        cpu_message_type m_type;

        //! Raw data, usually specified by message type
        std::vector<std::uint8_t> m_data;

        //! This callback is called when an error occurs, see message type
        std::function<void(void)> m_on_error;
    };

    //! A function of this type is called when the CPU receives a message
    using cpu_message_handler = std::function<void(const cpu_message &)>;

    //! @brief          Send a message to the cpu thread
    //! @param message  The cpu_message structure
    void send_message(const cpu_message &);

    //! @brief      Register a message handler to be called in the cpu thread when it receives a message
    //! @param type Message type
    //! @param hdl  Message handler container
    void register_message_handler(const cpu_message_type &type, const cpu_message_handler &hdl);

    //! CPU state enumeration
    enum cpu_state
    {
        paused,
        running
    };

    //! @brief Get cpu_state
    //! @returns cpu_state
    cpu_state get_cpu_state() const;

    //! @brief Set cpu_state
    void set_cpu_state(const cpu_state &);

    std::vector<bool> &get_screen();

private:
    //! CPU instance
    cpu m_cpu;

    cpu_state m_cpu_state;

    std::thread m_cpu_thread;

    void cpu_thread();

    //! Locked when a message is being passed, or handled
    std::mutex m_cpu_thread_mutex;

    std::queue<cpu_message> m_unhandled_messages;

    //! Message handlers, first indexed by type, and then by each handler for that type
    std::vector<std::vector<cpu_message_handler>> m_message_handlers;
};

}
#endif //CHIP8_NCURSES_CPU_DAEMON_HPP
