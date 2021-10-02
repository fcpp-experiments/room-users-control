cmake_minimum_required(VERSION 3.9)
option(FCPP_INTERNAL_TESTS "Build internal tests for FCPP." OFF)
add_subdirectory(./fcpp/src)
fcpp_setup()

project(
    room_users_control
    VERSION 0.0.0
    DESCRIPTION "Case study on users control and satisfaction in a room."
)

fcpp_target(graphic ./run/graphic.cpp ON)
