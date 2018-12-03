cmake_minimum_required(VERSION 2.8)

set(SOURCES "main.cpp" "TcpListener.cpp")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

project(ws_server)
add_executable(${PROJECT_NAME} ${SOURCES})
