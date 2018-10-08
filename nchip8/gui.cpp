//
// Created by ocanty on 26/09/18.
//

#include "gui.hpp"

#include <curses.h>
#include <clocale>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

gui::gui()
{
    this->rebuild_windows();
    this->start_stream_redirection();
    // std::cout << "[gui] init ncurses" << std::endl;
    this->thread();
}

gui::~gui()
{
    this->end_stream_redirection();
}

void gui::rebuild_windows()
{
    ::newterm(NULL, NULL, stdin);

    m_window = std::shared_ptr<::WINDOW>(::initscr(), ::wdelch);
    ::setlocale(LC_ALL," ");
    ::cbreak();
    ::noecho();
    ::nonl();
    ::intrflush(stdscr, FALSE);
    ::keypad(stdscr, TRUE);

    ::wborder(m_window.get(),0,0,0,0,0,0,0,0);
    getmaxyx(m_window.get(),m_window_w,m_window_h);

    m_screen_window = std::shared_ptr<::WINDOW>(::newwin(16+2,64+2,0,1));
    ::wborder(m_screen_window.get(), 0,0,0,0,0,0,0,0);
    ::wrefresh(m_screen_window.get());


    m_log_window = std::shared_ptr<::WINDOW>(::newwin(8+2,64+2,18,1), ::wdelch);
    ::wborder(m_log_window.get(),0,0,0,0,0,0,0,0);
    ::wrefresh(m_log_window.get());
    this->update_log_window();
}

void gui::update_windows_on_resize()
{
    // get current terminal size
    int new_term_w = 0;
    int new_term_h = 0;
    getmaxyx(m_window.get(),new_term_w,new_term_h);

    // resize the window if it's changed within the past update
    if(new_term_w != m_window_w || new_term_h != m_window_h)
    {
        m_window_w = new_term_w; m_window_h = new_term_h;
        this->rebuild_windows();
    }
}

void gui::thread()
{
    bool die = false;
    while(!die)
    {
        update_windows_on_resize();
        update_logs_on_cout();

        // dont eat cpu
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void gui::update_logs_on_cout()
{
    if(!m_cout_ss.str().empty())
    {
        while(m_cout_ss.good() && !m_cout_ss.eof())
        {
            static std::string line;

            line.clear();
            std::getline(m_cout_ss, line);
            m_log.push_back(line);
        }

        this->update_log_window();
    }

}

void gui::update_log_window()
{
    if(m_log_window == nullptr) return;


    //::wborder(m_log_window.get(), 0,0,0,0,0,0,0,0);

    //::wrefresh(m_log_window.get());
}

void gui::start_stream_redirection()
{
    // get the read buffers of the cout,cerr streams if we had previously not redirected it
    if(m_cout_rdbuf == nullptr)
    {
        std::streambuf* rdbuf = std::cout.rdbuf();
        m_cout_rdbuf = std::shared_ptr<std::streambuf>(rdbuf);
    }

    if(m_cerr_rdbuf == nullptr)
    {
        std::streambuf* rdbuf = std::cerr.rdbuf();
        m_cerr_rdbuf = std::shared_ptr<std::streambuf>(rdbuf);
    }

    // set the cout,cerr read bufs to our string streams
    std::cout.rdbuf(m_cout_ss.rdbuf());
    std::cerr.rdbuf(m_cerr_ss.rdbuf());
}

void gui::end_stream_redirection()
{
    // if we have the stream buffers stored, set them back
    if(m_cout_rdbuf != nullptr) std::cout.rdbuf(m_cout_rdbuf.get());
    if(m_cerr_rdbuf != nullptr) std::cerr.rdbuf(m_cerr_rdbuf.get());
}
