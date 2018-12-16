//
// Created by ocanty on 16/12/18.
//

#include "cpu_message.hpp"

namespace nchip8
{

cpu_message::cpu_message(const cpu_message_type& type) :
m_type(type),
m_data({}),
m_callback([](){}),
m_on_error([](){})
{

};

cpu_message::cpu_message(const cpu_message_type& type,
            std::vector<std::uint8_t> data) :
m_type(type),
m_data(std::move(data)),
m_callback([](){}),
m_on_error([](){})
{

};

cpu_message::cpu_message(const cpu_message_type& type,
            std::vector<std::uint8_t> data,
            std::function<void(void)> callback) :
m_type(type),
m_data(std::move(data)),
m_callback(std::move(callback)),
m_on_error([](){})
{

};

cpu_message::cpu_message(const cpu_message_type& type,
            std::vector<std::uint8_t> data,
            std::function<void(void)> callback,
            std::function<void(void)> error) :
m_type(type),
m_data(std::move(data)),
m_callback(std::move(callback)),
m_on_error(std::move(error))
{

};

}