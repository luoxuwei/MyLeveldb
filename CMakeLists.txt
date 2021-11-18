cmake_minimum_required(VERSION 3.17)
project(MyLeveldb)

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set(CMAKE_CXX_STANDARD 14)
set(LEVELDB_PUBLIC_INCLUDE_DIR include/leveldb)

include_directories(./include)
include_directories(./include/leveldb)
include_directories(./util)

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} util/*.cc util/*.h)
source_group(util FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} include/leveldb/*.cc include/leveldb/*.h)
source_group(util FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} db/*.cc db/*.h)
source_group(util FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

set(SRCFILES ${SELF_SRC_FILES})

add_library(myleveldb SHARED
        ${SRCFILES})


