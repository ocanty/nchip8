//
// Created by ocanty on 26/09/18.
//

#include "io.hpp"
#include "gui.hpp"

#include <curses.h>
#include <clocale>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>

namespace nchip8
{

gui::gui(const std::shared_ptr<cpu_daemon>& cpu) :
    m_cpu_daemon(cpu)
{
    this->rebuild_windows();
    this->loop();
}

gui::~gui()
{
    // clenaup curses
    ::curs_set(1);
    clear();
    ::endwin();
}

void gui::rebuild_windows()
{

    nchip8::log << "[gui] rebuilt windows" << '\n';

    ::setlocale(LC_ALL, ""); // set locale, needs to be done before ncurses init

    m_window = std::shared_ptr<::WINDOW>(::initscr(), ::wdelch);
    ::cbreak();
    ::noecho();
    ::nonl();
    ::intrflush(stdscr, FALSE);
    ::keypad(stdscr, TRUE);
    ::curs_set(0); // disable cursor

    // 66 x 18 (64x16 excluding border)
    m_screen_window = std::shared_ptr<::WINDOW>(::newwin(18, 66, 0, 0), ::wdelch);
    ::wborder(m_screen_window.get(), 0, 0, 0, 0, 0, 0, 0, 0);
    ::wrefresh(m_screen_window.get());

    m_log_window = std::shared_ptr<::WINDOW>(::newwin(10, 66, 18, 0), ::wdelch);
    ::wborder(m_log_window.get(), 0, 0, 0, 0, 0, 0, 0, 0);
    ::wrefresh(m_log_window.get());
    this->update_log_window();

    m_reg_window = std::shared_ptr<::WINDOW>(::newwin(28, 14, 0, 66), ::wdelch);
    ::wborder(m_reg_window.get(), 0, 0, 0, 0, 0, 0, 0, 0);
    ::wrefresh(m_reg_window.get());
}

void gui::update_windows_on_resize()
{
    // get current terminal size
    static int new_term_w = 0;
    static int new_term_h = 0;
    getmaxyx(m_window.get(), new_term_h, new_term_w);

    // resize the window if it's changed its size within the past update
    if (new_term_w != m_window_w || new_term_h != m_window_h)
    {
        m_window_w = new_term_w;
        m_window_h = new_term_h;
        this->rebuild_windows();
    }
}

void gui::loop()
{
    bool die = false;
    while (!die)
    {
        //update_windows_on_resize();
        update_log_on_global_log_change();
        update_screen_window();
        update_reg_window();

        // dont eat cpu
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void gui::update_log_on_global_log_change()
{
    if (!nchip8::log.str().empty())
    {
        static std::string line;

        while (std::getline(nchip8::log, line) && !line.empty())
        {
            m_gui_log.push_back(line);
            line.empty();
        }

        nchip8::log.str(""); nchip8::log.clear();
        this->update_log_window();
    }

}

void gui::update_log_window()
{
    if (m_log_window == nullptr) return;

    // get height of log window and get its height minus its borders
    int log_window_w, log_window_h = 0;
    getmaxyx(m_log_window.get(), log_window_h, log_window_w);

    int height = log_window_h - 1; // height without borders
    int y = height;// initial y coordinate (from bottom) (skipping bottom border)

    // find out how many lines we need to draw, its either the max window height if it fits fully
    // or the size that doesnt
    auto draw_size = (m_gui_log.size() >= height ? height : m_gui_log.size());
    for(auto it = m_gui_log.rbegin(); it != (m_gui_log.rbegin()+draw_size); it++)
    {
        // clear the line
        ::mvwprintw(m_log_window.get(), y, 1, std::string(64,' ').c_str());

        // draw log
        ::mvwprintw(m_log_window.get(), y, 1, (*it).c_str());
        y--;
    }

    ::wborder(m_log_window.get(), 0, 0, 0, 0, 0, 0, 0, 0);
    ::wrefresh(m_log_window.get());
}

void gui::update_screen_window()
{
    if (!m_cpu_daemon || !m_screen_window)
    {
        return;
    }

    auto mode = m_cpu_daemon->get_screen_mode();

    // We need to convert screen pixels to block level elements
    // It's important to realise that we are not simply representing a pixel by 1 block
    // We are compressing the 2 rows of pixels into 1 line of characters
    // ▄ to represent the top pixel being off, the bottom on

    // ▀ to represent the bottom pixel being off, the top on
    // █ to represent 2 pixels above each-other

    unsigned int width = (mode == cpu::screen_mode::hires_sc8 ? 128 : 64);
    unsigned int height = (mode == cpu::screen_mode::lores_c8 ? 64 : 32);

    unsigned int draw_y = 1; // start at 1 to avoid border of the window
    std::wstring line;

    for (unsigned int y = 0; y < height; y += 2)
    {
        line.clear();

        for (unsigned int x = 0; x < width; x++)
        {
            bool set_top = m_cpu_daemon->get_screen_xy(x, y);

            unsigned int nextrow_y = y + 1;
            bool set_bottom = m_cpu_daemon->get_screen_xy(x, nextrow_y);

            if (set_top && set_bottom) { line += L"█"; /* █ */ continue; }
            if (set_top)               { line += L"▀"; /* ▀ */ continue; }
            if (set_bottom)            { line += L"▄"; /* ▄ */ continue; }
            line += '.';
        }
        mvwaddwstr(m_screen_window.get(), draw_y, 1, line.c_str());

        draw_y++;
    }

    ::wrefresh(m_screen_window.get());
}

void gui::update_reg_window()
{
    if(!m_cpu_daemon || !m_reg_window){ return; }
    std::stringstream row;

    for(int i = 0; i < 16; i++)
    {
        row << nchip8::V << i << " " << std::hex << (std::uint16_t)m_cpu_daemon->get_gpr().at(i);
        mvwaddstr(m_reg_window.get(), i+1, 1, row.str().c_str());
        row.str(""); row.clear();
    }

    row << "PC " << nchip8::nnn << (std::uint16_t)m_cpu_daemon->get_pc();
    mvwaddstr(m_reg_window.get(), 18, 1, row.str().c_str());
    row.str(""); row.clear();

    row << "SP " << nchip8::nnn << (std::uint16_t)m_cpu_daemon->get_sp();
    mvwaddstr(m_reg_window.get(), 19, 1, row.str().c_str());
    row.str(""); row.clear();

    row << " I" << nchip8::nnn << (std::uint16_t)m_cpu_daemon->get_i();
    mvwaddstr(m_reg_window.get(), 20, 1, row.str().c_str());
    row.str(""); row.clear();


    ::wrefresh(m_reg_window.get());
}

}