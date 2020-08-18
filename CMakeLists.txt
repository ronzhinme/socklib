
cmake_minimum_required(VERSION 2.6)

IF(DEFINED CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "None Debug 
Release RelWithDebInfo MinSizeRel.")
ELSE()
   SET(CMAKE_BUILD_TYPE Debug CACHE STRING "None Debug 
Release RelWithDebInfo MinSizeRel.")
ENDIF()

set(WITH_UNITTEST 0)

if(WITH_UNITTEST)
# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
endif()

if (WIN32)
   set(CMAKE_CXX_FLAGS_RELEASE "/MD")
   set(CMAKE_CXX_FLAGS_DEBUG "/MDd")
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
   set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
   set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")

   #SocketLib
   add_library(socklib STATIC SockSample/BytesData.cpp SockSample/Socket_win.cpp)
   target_compile_definitions(socklib PUBLIC WIN32)

   if(WITH_UNITTEST)
      # UnitTests
      include_directories(SockSample)
      add_executable(Socket_unittests Socket_Unittests/Socket_Unittests.cpp)
      target_compile_definitions(Socket_unittests PUBLIC WIN32)
      target_link_libraries(Socket_unittests socklib ${GTEST_LIBRARIES})
   endif()

elseif (UNIX)
   #SocketLib
   add_library(socklib STATIC SockSample/BytesData.cpp SockSample/Socket_linux.cpp)
   target_compile_definitions(socklib PUBLIC LINUX)

   if(WITH_UNITTEST)
      # UnitTests
      add_executable(Socket_unittests Socket_Unittests/Socket_Unittests.cpp)
      target_compile_definitions(Socket_unittests PUBLIC LINUX)
      target_link_libraries(Socket_unittests socklib)
      target_link_libraries(Socket_unittests ${GTEST_LIBRARIES} pthread)
   endif()

elseif (APPLE)
endif()
