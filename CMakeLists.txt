cmake_minimum_required( VERSION 3.27 )
project( sudoku LANGUAGES CXX)

add_executable( sudoku main.cpp)
set_property(TARGET sudoku PROPERTY CXX_STANDARD 23)
add_compile_options(-g -Wall -Wextra -pedantic)

set(CURSES_NEED_WIDE true)
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES})

install( TARGETS sudoku
         RUNTIME DESTINATION bin )