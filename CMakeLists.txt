cmake_minimum_required(VERSION 2.8)
project(ws_server)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
set(SOURCES "src/main.cpp" "src/socket_listener.cpp" "src/task_queue.cpp")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -std=c++17 -pthread")
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE
  "headers"
)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

