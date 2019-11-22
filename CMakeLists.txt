cmake_minimum_required(VERSION 2.8)
project(ws_server)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
set(SOURCES "main.cpp" "socket_listener.cpp")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++17 -g -pthread")
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE
  "headers"
)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

