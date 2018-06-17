
cmake_minimum_required(VERSION 2.6)

IF(DEFINED CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "None Debug 
Release RelWithDebInfo MinSizeRel.")
ELSE()
   SET(CMAKE_BUILD_TYPE Debug CACHE STRING "None Debug 
Release RelWithDebInfo MinSizeRel.")
ENDIF()

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

#SocketLib
add_library(socklib STATIC SockSample/BytesData.cpp SockSample/Socket_linux.cpp)
target_compile_definitions(socklib PUBLIC LINUX)

# UnitTests
add_executable(Socket_unittests Socket_Unittests/Socket_Unittests.cpp)
target_compile_definitions(Socket_unittests PUBLIC LINUX)
target_link_libraries(Socket_unittests socklib)
target_link_libraries(Socket_unittests ${GTEST_LIBRARIES} pthread)