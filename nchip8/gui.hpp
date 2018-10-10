//
// Created by ocanty on 26/09/18.
//

#ifndef CHIP8_NCURSES_GUI_HPP
#define CHIP8_NCURSES_GUI_HPP

#include <curses.h>
#include <sstream>
#include <vector>
#include <memory>

namespace nchip8
{

class gui
{
public:
    gui();

    ~gui();

private:
    //! @brief This should block the main thread, update drawings for view
    void loop();

    //! Main window width
    int m_window_w = 0;

    //! Main window height
    int m_window_h = 0;

    std::shared_ptr<::WINDOW> m_window          = nullptr;
    std::shared_ptr<::WINDOW> m_screen_window   = nullptr;
    std::shared_ptr<::WINDOW> m_log_window      = nullptr;

    //! @brief  Rebuilds window when a size change is detected
    void update_windows_on_resize();


    //! The local, gui log (the one drawn by the gui)
    std::vector<std::string> m_gui_log;

    //! @brief  Checks if data has been written to the global log,
    //!         pushes it to gui log and redraws the window
    void update_log_on_global_log_change();

    void update_log_window();

    //! @brief Redraw's all the windows to the current terminal height and width
    void rebuild_windows();

};

}

#endif //CHIP8_NCURSES_GUI_HPP
