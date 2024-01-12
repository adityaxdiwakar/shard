cmake_minimum_required(VERSION 3.10)
project(shard)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CPP_FILES 
    src/main.cc)

set(HEADER_FILES)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(shard ${CPP_FILES} ${HEADER_FILES})
target_link_libraries(shard ${Boost_LIBRARIES})
target_include_directories(shard PRIVATE include/)
