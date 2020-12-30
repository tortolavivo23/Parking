cmake_minimum_required(VERSION 3.17)
project(Parking C)

set(CMAKE_C_STANDARD 11)

add_executable(Parking main.c)

option(CMAKE_USE_WIN32_THREADS_INIT "using WIN32 threads" ON)
option(gtest_disable_pthreads "Disable uses of pthreads in gtest." ON)