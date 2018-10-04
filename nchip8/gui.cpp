//
// Created by ocanty on 26/09/18.
//

#include "gui.hpp"

#include <curses.h>
#include <clocale>

gui::gui()
{
    this->rebuild_windows();
    this->thread();
}

gui::~gui()
{

}

void gui::rebuild_windows()
{
    if(m_window != NULL)        ::wdelch(m_window);
    if(m_log_window != NULL)    ::wdelch(m_log_window);
    if(m_screen_window != NULL) ::wdelch(m_screen_window);

    ::setlocale(LC_ALL," ");
    m_window = ::initscr();
    ::cbreak();
    ::noecho();
    ::nonl();
    ::intrflush(stdscr, FALSE);
    ::keypad(stdscr, TRUE);

    getmaxyx(m_window,m_window_w,m_window_h);

    m_screen_window = ::newwin(16+2,64+2,0,1);

    const auto& remaining_w = (m_window_w - (32+4)); // +2 for border
    const auto& remaining_h = (m_window_h - (16+2)); // +2 for border
    m_log_window = ::newwin(remaining_h,remaining_w,16+2,1);

    ::wclear(m_window);
    ::wclear(m_screen_window);
    ::wclear(m_log_window);

    ::wborder(m_window,0,0,0,0,0,0,0,0);
    ::wborder(m_screen_window,0,0,0,0,0,0,0,0);
    ::wborder(m_log_window,0,0,0,0,0,0,0,0);

    ::wrefresh(m_log_window);
    ::wrefresh(m_window);
}

void gui::thread()
{
    bool die = false;
    while(!die)
    {
        // get current terminal size
        int new_term_w = 0;
        int new_term_h = 0;
        getmaxyx(m_window,new_term_w,new_term_h);

        // resize the window if it's changed
        if(new_term_w != m_window_w || new_term_h != m_window_h)
        {
            m_window_w = new_term_w; m_window_h = new_term_h;
            this->rebuild_windows();
        }


    }
}