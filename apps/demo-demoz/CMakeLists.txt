project(demo-demoz VERSION 0.1.5)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

cmake_minimum_required(VERSION 3.5)

add_executable(demo-demoz main.cpp)

target_link_libraries(demo-demoz PRIVATE colors image)

find_package(OpenMP)

if (OpenMP_FOUND OR OpenMP_CXX_FOUND)
   target_link_libraries(demo-demoz PRIVATE OpenMP::OpenMP_CXX)
endif()
