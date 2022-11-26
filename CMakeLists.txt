cmake_minimum_required(VERSION 3.23)

project(RaidMonitorServer)

set(CMAKE_CXX_STANDARD 17)

add_executable(RaidMonitorServer main.cpp
        Monitor.cpp Monitor.h
        MemoryMonitor.cpp MemoryMonitor.h
        CPUMonitor.cpp CPUMonitor.h
        DiskMonitor.cpp DiskMonitor.h
        Common.h)


message(STATUS "path is = ${CMAKE_CURRENT_SOURCE_DIR}")

set(ZeroMQ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/share/zeromq)
set(cppzmq_DIR ${CMAKE_CURRENT_SOURCE_DIR}/share/cppzmq)

find_package(ZeroMQ CONFIG REQUIRED)
find_package(cppzmq CONFIG REQUIRED)


include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/nlohmann_json)
FetchContent_MakeAvailable(json)

FetchContent_Declare(spdlog URL https://github.com/gabime/spdlog/archive/refs/tags/v1.11.0.tar.gz
SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/spdlog)
FetchContent_MakeAvailable(spdlog)

target_link_libraries(${PROJECT_NAME} PRIVATE  -static-libstdc++ nlohmann_json::nlohmann_json spdlog::spdlog cppzmq-static)

add_executable(Client ClientTest.cpp raid_monitor_server.h)
target_link_libraries(Client PRIVATE cppzmq-static)