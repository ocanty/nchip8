//
// Created by ocanty on 26/09/18.
//

#ifndef CHIP8_NCURSES_GUI_HPP
#define CHIP8_NCURSES_GUI_HPP

#include <curses.h>
#include <sstream>
#include <vector>
#include <memory>

#include "cpu_daemon.hpp"

namespace nchip8
{

class gui
{
public:
    //! @brief Constructor
    //!
    //! @param cpu  CPU
    //!             that the GUI will display the screen, disassembly & status of
    gui(const std::shared_ptr<cpu_daemon>& cpu);

    ~gui();

    //! @brief Start the GUI logic thread, this will block input and the main thread!
    void loop();

private:
    std::shared_ptr<cpu_daemon> m_cpu_daemon;

    //! Main window width
    int m_window_w = 0;

    //! Main window height
    int m_window_h = 0;

    //! Pointers to the ncurses windows
    std::shared_ptr<::WINDOW> m_window          = nullptr;
    std::shared_ptr<::WINDOW> m_screen_window   = nullptr;
    std::shared_ptr<::WINDOW> m_log_window      = nullptr;
    std::shared_ptr<::WINDOW> m_reg_window      = nullptr;

    //! @brief  Rebuilds window when a size change is detected
    void update_windows_on_resize();

    //! The local, gui log (the one drawn by the gui)
    std::vector<std::string> m_gui_log;

    //! @brief  Checks if data has been written to the global log,
    //!         pushes it to m_gui_log and redraws the window
    void update_log_on_global_log_change();

    //! @brief Draws log from m_gui_log
    void update_log_window();

    //! @brief Updates the screen if the gui is attached to a cpu_daemon
    void update_screen_window();

    //! @brief  Update the register preview window, showing all the values of the CPU registers
    void update_reg_window();

    //! @brief Redraw's all the windows to the current terminal height and width
    void rebuild_windows();

};

}

#endif //CHIP8_NCURSES_GUI_HPP
