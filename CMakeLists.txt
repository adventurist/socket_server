cmake_minimum_required(VERSION 2.8)
project(ws_server)

set(SOURCES "main.cpp" "socket_listener.cpp")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -std=c++17")
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE
  "headers"
)

