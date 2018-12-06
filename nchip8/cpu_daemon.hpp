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

    virtual ~cpu_daemon();

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
    public:
        explicit cpu_message(const cpu_message_type& type) :
        m_type(type),
        m_data({}),
        m_callback([](){}),
        m_on_error([](){})
        { };

        cpu_message(const cpu_message_type& type,
                    std::vector<std::uint8_t> data) :
        m_type(type),
        m_data(std::move(data)),
        m_callback([](){}),
        m_on_error([](){})
        { };

        cpu_message(const cpu_message_type& type,
                    std::vector<std::uint8_t> data,
                    std::function<void(void)> callback) :
        m_type(type),
        m_data(std::move(data)),
        m_callback(std::move(callback)),
        m_on_error([](){})
        { };

        cpu_message(const cpu_message_type& type,
                    std::vector<std::uint8_t> data,
                    std::function<void(void)> callback,
                    std::function<void(void)> error) :
        m_type(type),
        m_data(std::move(data)),
        m_callback(std::move(callback)),
        m_on_error(std::move(error))
        { };

        //! See cpu_message_type
        cpu_message_type m_type;

        //! Raw data, usually specified by message type
        std::vector<std::uint8_t> m_data;

        //! Called when the message has finished processing
        //! (this is called in the cpu_thread!)
        std::function<void(void)> m_callback;

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

    //! @brief Returns current screen mode
    //! @see cpu::screen_mode
    const cpu::screen_mode& get_screen_mode() const;

    //! @brief      Returns a reference to screen data
    //! @returns    Const vector reference that contains the screen data
    //!             (where true = pixel on, false = pixel off)
    //! @details    Screen array is ALWAYS the hires size, even if cpu is
    const std::array<bool, 128*64>& get_screen_framebuffer() const;

    //! @brief Get's the status of a pixel on the screen (on/off)
    bool get_screen_xy(const std::uint8_t&x , const std::uint8_t& y) const;

    void set_key_down(const std::uint8_t& key);
    void set_key_down_none();

    //! @brief Returns a reference to the general purpose cpu registers (i.e V0-V15)
    const std::array<std::uint8_t, 16>& get_gpr() const;

    //! @brief Get I register
    const std::uint16_t get_i() const;

    //! @brief Get Stack Pointer
    const std::uint16_t get_sp() const;

    //! @brief Get Program Counter
    const std::uint16_t get_pc() const;

    //! @brief Get Delay Timer
    const std::uint8_t get_dt() const;

    //! @brief Get Sound Timer
    const std::uint8_t get_st() const;

    //! @brief Get stack
    const std::array<std::uint16_t, 16> get_stack() const;



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
