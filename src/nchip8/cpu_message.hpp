//
// Created by ocanty on 16/12/18.
//

#ifndef NCHIP8_CPU_MESSAGE_HPP
#define NCHIP8_CPU_MESSAGE_HPP

#include <functional>
#include <vector>

namespace nchip8
{

//! @brief Message types supported by cpu_daemon
enum cpu_message_type : std::uint8_t
{
    Reset,              //! Resets the cpu. Clear registers & ram, PC = 0x200   m_data: none
    LoadROM,            //! Writes a rom to cpu memory.                         m_data: vector of ROM binary
    SetStateRunning,    //! Set cpu running
    SetStatePaused,     //! Pause cpu at current instruction
    _last               // Used to find amount of messages, keep at end of enum
};

//! @brief Message container for messages passed to cpu_daemon
class cpu_message
{
public:
    //! @brief Construct a message based only on its type
    explicit cpu_message(const cpu_message_type& type);

    //! @brief      Construct a message based on it's type and pass data
    //! @param data See cpu_message_type for info on what data can be passed
    cpu_message(const cpu_message_type& type,
                std::vector<std::uint8_t> data);

    //! @brief          Construct a message based on it's type and pass data
    //!                 with a callback when the message is processed
    //! @param data     See cpu_message_type for info on what data can be passed
    //! @param callback lamdba
    cpu_message(const cpu_message_type& type,
                std::vector<std::uint8_t> data,
                std::function<void(void)> callback);


    //! @brief          Construct a message based on it's type and pass data
    //!                 with a callback when the message is processed, or an
    //!                 error callback when the message is invalid
    //! @param data     See cpu_message_type for info on what data can be passed
    //! @param callback lamdba
    //! @param error    lamdba
    cpu_message(const cpu_message_type& type,
                std::vector<std::uint8_t> data,
                std::function<void(void)> callback,
                std::function<void(void)> error);

    //! @see cpu_message_type
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
}

#endif //NCHIP8_CPU_MESSAGE_HPP
