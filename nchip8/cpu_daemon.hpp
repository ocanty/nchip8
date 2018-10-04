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

class cpu_daemon
{
public:
    cpu_daemon();
    ~cpu_daemon();

    enum cpu_message_type : std::uint8_t
    {
        Reset,
        LoadROM,
        SetStateRunning,
        SetStatePaused,
        _last
    };

    struct cpu_message
    {
        cpu_message_type m_type;
        std::vector<std::uint8_t> m_data;
        std::function<void(void)> m_on_error;
    };

    using cpu_message_handler = std::function<void(const cpu_message&)>;

    void send_message(const cpu_message&);
    void register_message_handler(const cpu_message_type&, const cpu_message_handler&);

    enum cpu_state
    {
        paused,
        running,
        steponce
    };

    cpu_state get_cpu_state();
    void set_cpu_state(const cpu_state&);

private:
    cpu_state m_cpu_state;

    std::thread m_cpu_thread;
    void cpu_thread();

    std::mutex m_cpu_thread_mutex;

    std::queue<cpu_message> m_unhandled_messages;

    std::vector<std::vector<cpu_message_handler>> m_message_handlers;

    cpu m_cpu;
};


#endif //CHIP8_NCURSES_CPU_DAEMON_HPP
