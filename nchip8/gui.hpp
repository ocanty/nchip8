//
// Created by ocanty on 26/09/18.
//

#ifndef CHIP8_NCURSES_GUI_HPP
#define CHIP8_NCURSES_GUI_HPP

#include <curses.h>
#include <sstream>
#include <vector>
#include <memory>

class gui
{
public:
    gui();
    ~gui();

private:
    //! @brief Enable redirection of cout, cerr to string streams
    void start_stream_redirection();

    //! @brief Disable redirection to string streams
    void end_stream_redirection();
    std::stringstream m_cout_ss;
    std::stringstream m_cerr_ss;
    std::shared_ptr<std::streambuf> m_cout_rdbuf = nullptr;
    std::shared_ptr<std::streambuf> m_cerr_rdbuf = nullptr;

    // Each line in m_cout_ss is pushed in m_log
    std::vector<std::string> m_log;

    //! @brief This should block the main thread, update drawings for view
    void thread();

    int m_window_w = 0;
    int m_window_h = 0;
    std::shared_ptr<::WINDOW> m_window         = nullptr;
    std::shared_ptr<::WINDOW> m_screen_window  = nullptr;
    std::shared_ptr<::WINDOW> m_log_window     = nullptr;

    //! @brief  Rebuilds window when a size change is detected
    void update_windows_on_resize();

    //! @brief  Checks if data has been written to the redirected stringstreams,
    //!         pushes it to the log and redraws the window
    void update_logs_on_cout();

    void update_log_window();

    //! @brief Redraw's all the windows to the current terminal height and width
    void rebuild_windows();

};


#endif //CHIP8_NCURSES_GUI_HPP
