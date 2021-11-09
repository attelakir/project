cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(project)

SET(GCC_COVERAGE_LINK_FLAGS    "-lpcap")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/")

include(FindPCAP.cmake)
find_package(PCAP REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC /usr/local/include/pcapplusplus)
target_link_libraries(${PROJECT_NAME} ${PCAP_LIBRARY} libCommon++.a libPacket++.a libPcap++.a)