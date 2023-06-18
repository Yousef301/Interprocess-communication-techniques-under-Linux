cmake_minimum_required(VERSION 3.25)
project(Project2 C)

set(CMAKE_C_STANDARD 99)

add_executable(Test main.c procChild.c Display.c sender.c receiver.c parent.c parent.c encoders.c parent.c parent.c parent.c helper.c spy.c master.c display.c)
