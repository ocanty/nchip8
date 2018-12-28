//
// Created by ocanty on 17/09/18.
//

#include "cpu_daemon.hpp"
#include "io.hpp"

namespace nchip8
{

cpu_daemon::cpu_daemon() :
    m_cpu_state(cpu_state::paused)
{
    // create enough space to hold the handlers for each type
    m_message_handlers.resize(cpu_message_type::_last);

    // handle rom loads
    this->register_message_handler(cpu_message_type::LoadROM, [this](const cpu_message &msg)
    {
        nchip8::log << "[cpu_daemon] received rom: " << msg.m_data.size() << " bytes " << '\n';

        // load rom in
        bool loaded = m_cpu.load_rom(msg.m_data, 0x200);

        if(loaded)
        {
            nchip8::log << "[cpu_daemon] rom loaded" << '\n';
            msg.m_callback();
            return;
        }

        msg.m_on_error();
    });

    this->register_message_handler(cpu_message_type::Reset, [this](const cpu_message &msg)
    {
        nchip8::log << "[cpu_daemon] reset cpu " << '\n';

        // reset cpu
        m_cpu.reset();
        msg.m_callback();

    });


    nchip8::log << "[cpu_daemon] starting cpu thread" << '\n';
    m_cpu_thread = std::thread(&cpu_daemon::cpu_thread, this);
}

cpu_daemon::~cpu_daemon()
{
    m_cpu_thread.join();
}

cpu_daemon::cpu_state cpu_daemon::get_cpu_state() const
{
    return m_cpu_state;
}

void cpu_daemon::set_cpu_state(const cpu_daemon::cpu_state &state)
{
    m_cpu_state = state;
}


void cpu_daemon::cpu_thread()
{
    bool die = false;

    while(!die)
    {
        if(m_cpu_state == cpu_state::running)
        {
            m_cpu.execute_op_at_pc();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / m_clock_speed));
        }

        std::unique_lock<std::mutex> lock(m_cpu_thread_mutex);
        while(!m_unhandled_messages.empty())
        {
            // get front of queue
            const auto &msg = m_unhandled_messages.front();

            // does the message have message handlers? is it of the correct type?
            if (!m_message_handlers.at(msg.m_type).empty())
            {

                // call all the message handlers
                // remember: using cpu_message_handler = std::function<void(const cpu_message &)>;
                for (cpu_message_handler &handler : m_message_handlers.at(msg.m_type))
                {
                    handler(msg);
                }
            }

            m_unhandled_messages.pop();
        }
    }
}

void cpu_daemon::send_message(const cpu_message &message)
{
    std::unique_lock<std::mutex> lock(m_cpu_thread_mutex);
    // push our message
    m_unhandled_messages.push(message);

}

void cpu_daemon::register_message_handler(const cpu_message_type &type, const cpu_message_handler &hdl)
{
    // add a message handler to the message handlers list of the type
    m_message_handlers.at(type).push_back(hdl);
}

const cpu::screen_mode &cpu_daemon::get_screen_mode() const
{
    return m_cpu.get_screen_mode();
}

const std::array<bool, 128 * 64> &cpu_daemon::get_screen_framebuffer() const
{
    return m_cpu.get_screen_framebuffer();
}

bool cpu_daemon::get_screen_xy(const std::uint8_t &x, const std::uint8_t &y) const
{
    return m_cpu.get_screen_xy(x,y);
}

const std::array<std::uint8_t, 16>& cpu_daemon::get_gpr() const
{
    return m_cpu.m_gpr;
}

const std::uint16_t cpu_daemon::get_i() const
{
    return m_cpu.m_i;
}

const std::uint16_t cpu_daemon::get_sp() const
{
    return m_cpu.m_sp;
}

const std::uint16_t cpu_daemon::get_pc() const
{
    return m_cpu.m_pc;
}

const std::array<std::uint16_t, 16> cpu_daemon::get_stack() const
{
    return m_cpu.m_stack;
}

const std::uint8_t cpu_daemon::get_dt() const
{
    return m_cpu.m_dt;
}

const std::uint8_t cpu_daemon::get_st() const
{
    return m_cpu.m_st;
}

void cpu_daemon::set_key_down(const std::uint8_t &key)
{
    m_cpu.set_key_down(key);
}

void cpu_daemon::set_key_up(const std::uint8_t &key)
{
    m_cpu.set_key_up(key);
}

void cpu_daemon::set_cpu_clockspeed(const size_t &speed)
{
    nchip8::log << "[cpu_daemon] set clock speed to " << std::dec << speed << "Hz " << std::endl;
    m_clock_speed = speed;
}

}