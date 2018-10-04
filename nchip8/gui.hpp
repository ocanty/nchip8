//
// Created by ocanty on 26/09/18.
//

#ifndef CHIP8_NCURSES_GUI_HPP
#define CHIP8_NCURSES_GUI_HPP

#include <curses.h>

class gui
{
public:
    gui();

    ~gui();

private:
    //! @brief Redraw's all the windows to the current terminal height and width
    void rebuild_windows();

    void thread();

    int m_window_w = 0;
    int m_window_h = 0;
    ::WINDOW* m_window = NULL;
    ::WINDOW* m_screen_window = NULL;
    ::WINDOW* m_log_window = NULL;

};


#endif //CHIP8_NCURSES_GUI_HPP
