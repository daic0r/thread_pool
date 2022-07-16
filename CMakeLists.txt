cmake_minimum_required(VERSION 3.21)

project(thread_pool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_BUILD_TYPE Debug)
find_package(Threads REQUIRED)
find_package(benchmark REQUIRED)

add_executable(${PROJECT_NAME} thread_pool.cpp main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads PRIVATE benchmark::benchmark)
