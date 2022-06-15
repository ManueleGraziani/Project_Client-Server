cmake_minimum_required(VERSION 3.15)
project(Client_TCP_C_ManueleGraziani C)

set(CMAKE_C_STANDARD 99)
LINK_LIBRARIES("pthread")

add_executable(Client_TCP_C_ManueleGraziani main.c)