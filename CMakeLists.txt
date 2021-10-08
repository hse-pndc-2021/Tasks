cmake_minimum_required(VERSION 3.15)
project(PnDC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

set(TASK "" CACHE STRING "Task id")
option(DISABLE_SANITIZERS "Disable address and thread sanitizer tests" OFF)

find_package(Threads REQUIRED)
include(cmake/Gtest.cmake)

add_subdirectory(library)

enable_testing()

function (make_task name)
    if (TASK STREQUAL name OR NOT TASK)
        add_subdirectory("${name}")
    endif()
endfunction()

function (make_test_executable name)
    add_executable("${name}" ${ARGN})
    target_link_libraries("${name}" PRIVATE Threads::Threads pndc)
    target_compile_options("${name}" PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Werror>)
    add_test(NAME "${name}" COMMAND "${name}")
    set_tests_properties("${name}" PROPERTIES TIMEOUT 30)
endfunction()

function (make_test name)
    make_test_executable("${name}" ${ARGN})
    target_link_libraries("${name}" PRIVATE gtest_main)

    if (NOT DISABLE_SANITIZERS)
        make_test_executable("${name}_ASAN" ${ARGN})
        target_link_libraries("${name}_ASAN" PRIVATE gtest_main)
        target_compile_options("${name}_ASAN" PRIVATE "-fsanitize=address")
        target_link_options("${name}_ASAN" PRIVATE "-fsanitize=address")
        target_compile_definitions("${name}_ASAN" PRIVATE TIMEOUT_SCALE_MULTIPLIER=3)

        make_test_executable("${name}_TSAN" ${ARGN})
        target_link_libraries("${name}_TSAN" PRIVATE gtest_main)
        target_compile_options("${name}_TSAN" PRIVATE "-fsanitize=thread")
        target_link_options("${name}_TSAN" PRIVATE "-fsanitize=thread")
        target_compile_definitions("${name}_TSAN" PRIVATE TIMEOUT_SCALE_MULTIPLIER=10)
    endif()
endfunction()

#################################################

make_task("blocking-queue")
make_task("mutex")
