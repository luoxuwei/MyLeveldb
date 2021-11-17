cmake_minimum_required(VERSION 3.17)
project(MyLeveldb)

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set(CMAKE_CXX_STANDARD 14)
set(LEVELDB_PUBLIC_INCLUDE_DIR include/leveldb)

include_directories(./include)
include_directories(./include/leveldb)

add_library(myleveldb SHARED
        ${LEVELDB_PUBLIC_INCLUDE_DIR}/slice.h
        ${LEVELDB_PUBLIC_INCLUDE_DIR}/status.h
        util/status.cc)

add_executable(MyLeveldb main.cpp)
target_link_libraries(MyLeveldb myleveldb)
