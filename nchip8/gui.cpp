//
// Created by ocanty on 26/09/18.
//

#include "io.hpp"
#include "gui.hpp"

#include <curses.h>
#include <clocale>
#include <cstdio>

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>
#include <unordered_map>

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

    ::nonl();
    ::intrflush(m_window.get(), FALSE);
    ::keypad(m_window.get(), TRUE);
    ::curs_set(0); // disable cursor

    // non blocking input
    ::cbreak();
    ::nodelay(m_window.get(), TRUE);
    ::noecho(); // dont draw chars when entered

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

/**
 *
 * Typical CHIP-8 keypad was to look like this:
    1	2	3	C
    4	5	6	D
    7	8	9	E
    A	0	B	F

    Let's do our best to map it to a modern keyboard
 */
const std::unordered_map<int, std::uint8_t> gui::key_mapping = {
    {'1',0x1}, {'2',0x2}, {'3',0x3}, {'4', 0xC},
    {'q',0x4}, {'w',0x5}, {'e',0x6}, {'r', 0xD},
    {'a',0x7}, {'s',0x8}, {'d',0x9}, {'f', 0xE},
    {'z',0xA}, {'x',0x0}, {'c',0xB}, {'v', 0xF},
};

void gui::loop()
{
    bool die = false;

    while (!die)
    {
        update_windows_on_resize();
        update_log_on_global_log_change();
        update_screen_window();
        update_reg_window();

        // we achieve multiple key inputs by giving each key a score
        // every frame we decrement the score
        // if the score is decremented, the key is no longer considered pressed

        // mappings are stored lowercase,
        // tolower will pass thru non-characters as-well (e.g. 0->0)
        int c = getch();
        int char_lowered = std::tolower(c);

        // if we didnt get a bad char and there is a valid mapping
        // pass it to the cpu
        if(c != ERR && key_mapping.count(char_lowered))
        {
            m_cpu_daemon->set_key_down(key_mapping.at(char_lowered));
        }
        else
        {
            m_cpu_daemon->set_key_down_none();
        }

        // gui aims to be at 60fps
        std::this_thread::sleep_for(std::chrono::milliseconds(1000/24));
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

    unsigned int draw_y = 1;

    std::wstring line;

    for (unsigned int y = 0; y < (height-1); y+=2)
    {
        for (unsigned int x = 0; x < width; x++)
        {
            bool set_top = m_cpu_daemon->get_screen_xy(x, y);

            // check the row of pixels below and see if we can get a group of two vertical pixels
            bool set_bottom = m_cpu_daemon->get_screen_xy(x, y + 1);

            if (set_top && set_bottom)
            { line += L"█"; /* █ */ continue; }
            if (set_top)
            { line += L"▀"; /* ▀ */ continue; }
            if (set_bottom)
            { line += L"▄"; /* ▄ */ continue; }
            line += ' ';

        }

        mvwaddwstr(m_screen_window.get(), draw_y, 1, line.c_str());

        line.clear();
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

    row << " I " << nchip8::nnn << (std::uint16_t)m_cpu_daemon->get_i();
    mvwaddstr(m_reg_window.get(), 20, 1, row.str().c_str());
    row.str(""); row.clear();

    row << "ST " << nchip8::nnn << (std::uint16_t)m_cpu_daemon->get_st();
    mvwaddstr(m_reg_window.get(), 21, 1, row.str().c_str());
    row.str(""); row.clear();

    row << "DT " << nchip8::nnn << (std::uint16_t)m_cpu_daemon->get_dt();
    mvwaddstr(m_reg_window.get(), 22, 1, row.str().c_str());
    row.str(""); row.clear();


    ::wrefresh(m_reg_window.get());
}

}