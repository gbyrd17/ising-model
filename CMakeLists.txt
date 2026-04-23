cmake_minimum_required(VERSION 3.20)

project(ising_sim VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(nlohmann_json 3.12.0 CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(OpenMP REQUIRED)

add_executable(ising_sim 
    src/main.cpp 
    src/simulator.cpp
)

target_include_directories(ising_sim PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)


target_link_libraries(ising_sim PRIVATE 
    nlohmann_json::nlohmann_json
    Catch2::Catch2
    Catch2::Catch2WithMain
    fmt::fmt
    OpenMP::OpenMP_CXX
)

if(MSVC)
    # windows-specific flags
    target_compile_options(ising_sim PRIVATE /W4 /MP /O2 /permissive-)
else()
    # linux/WSL flags 
    target_compile_options(ising_sim PRIVATE -Wall -Wextra -O3 -march=native)
endif()
