

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -lncursesw -std=c++17 -pthread")

find_package( PkgConfig REQUIRED )
pkg_check_modules ( ncurses++ REQUIRED ncurses++ )

add_executable(nchip8
        main.cpp
        nchip8/cpu.hpp
        nchip8/cpu.cpp
        nchip8/cpu_daemon.cpp
        nchip8/cpu_daemon.hpp
        nchip8/cpu.hpp
        nchip8/gui.cpp
        nchip8/gui.hpp
        nchip8/nchip8.cpp
        nchip8/nchip8.hpp
        nchip8/op_handlers.cpp nchip8/io.hpp nchip8/io.cpp nchip8/cpu_message.hpp nchip8/cpu_message.cpp)


target_link_libraries (nchip8 ${ncurses++_LIBRARIES} )