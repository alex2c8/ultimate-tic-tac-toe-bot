cmake_minimum_required(VERSION 3.5)
project(uttt_bot)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES uttt_bot.cpp)

add_executable(uttt_bot ${SOURCE_FILES})
