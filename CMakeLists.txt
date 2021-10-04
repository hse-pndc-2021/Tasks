cmake_minimum_required(VERSION 3.15)
project(PnDC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

set(TASK "" CACHE STRING "Task id")

find_package(Threads REQUIRED)
include(cmake/Gtest.cmake)

add_subdirectory(library)

enable_testing()

function (make_task name)
    if (TASK STREQUAL name OR NOT TASK)
        add_subdirectory("${name}")
    endif()
endfunction()

function (make_test name)
    add_executable("${name}" ${ARGN})
    target_link_libraries("${name}" PRIVATE Threads::Threads gtest_main pndc)
    target_compile_options("${name}" PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Werror>)
    add_test(NAME "${name}" COMMAND "${name}")
endfunction()

make_task("blocking-queue")
