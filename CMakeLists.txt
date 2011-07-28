cmake_minimum_required(VERSION 2.8)

project(mimosa C CXX)

enable_testing()

add_definitions(-D_GNU_SOURCE -D_LARGEFILE64_SOURCE -std=c++0x -Wall -Wextra)

add_subdirectory(mimosa)