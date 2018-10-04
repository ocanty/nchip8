//
// Created by ocanty on 17/09/18.
//

#include "cpu_daemon.hpp"
#include <iostream>

cpu_daemon::cpu_daemon() :
    m_cpu_state(cpu_state::paused)
{
    m_message_handlers.resize(cpu_daemon::cpu_message_type::_last);

    this->register_message_handler(cpu_message_type::LoadROM, [this](const cpu_message& msg)
    {
        // std::cout << "[cpu_daemon] received rom: " << msg.m_data.size() << " bytes " << std::endl;

        m_cpu.reset();
        m_cpu.load_rom(msg.m_data, 0x200);
    });

    this->register_message_handler(cpu_message_type::SetStateRunning,[this](const cpu_message& msg)
    {
        // std::cout << "set cpu running";
        this->set_cpu_state(cpu_state::running);
    });


    // std::cout << "[cpu_daemon] starting cpu thread" << std::endl;
    m_cpu_thread = std::thread(&cpu_daemon::cpu_thread, this);
}

cpu_daemon::~cpu_daemon()
{
    m_cpu_thread.join();
}

cpu_daemon::cpu_state cpu_daemon::get_cpu_state()
{
    return m_cpu_state;
}

void cpu_daemon::set_cpu_state(const cpu_daemon::cpu_state& state)
{
    m_cpu_state = state;
}


void cpu_daemon::cpu_thread()
{
    bool die = false;

    while(!die)
    {
        // if the message queue is not empty
        while(!m_unhandled_messages.empty())
        {
            // lock the data access to the message queue
            std::lock_guard<std::mutex> lock(m_cpu_thread_mutex);

            // get front of queue
            const auto& msg = m_unhandled_messages.front();

            // does the message have message handlers? is it of the correct type?
            if(m_message_handlers.at(msg.m_type).size() > 0)
            {
                // call all the message handlers
                for(auto& handler : m_message_handlers.at(msg.m_type))
                {
                    handler(msg);

                }
            }

            // dispose of the message
            m_unhandled_messages.pop();
        }

        if(this->get_cpu_state() == cpu_state::running)
        {
            m_cpu.execute_op_at_pc();
        }
    }
}

void cpu_daemon::send_message(const cpu_daemon::cpu_message& message)
{
    // prevent the cpu thread from reading/writing to the message queue
    std::lock_guard<std::mutex> lock(this->m_cpu_thread_mutex);

    // push our message
    m_unhandled_messages.push(message);
}

void cpu_daemon::register_message_handler(const cpu_message_type& type,const cpu_message_handler& hdl)
{
    // add a message handler to the message handlers list of the type
    m_message_handlers.at(type).push_back(hdl);
}
