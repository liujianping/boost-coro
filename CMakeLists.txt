#the whole CMakeLists.txt for project ttiger
PROJECT(coroutine)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

# include directories
INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}/
  /opt/local/include
)

# lib directories
LINK_DIRECTORIES(
  ${PROJECT_BINARY_DIR}/lib
  /opt/local/lib
)

MESSAGE(STATUS "OS IS:" ${CMAKE_SYSTEM_NAME})

IF(__APPLE__)
ADD_DEFINITIONS(-DDCORO_SJLJ=1)
ELSE(__APPLE__)
ADD_DEFINITIONS(-DCORO_ASM=1)
ENDIF(__APPLE__)

ADD_DEFINITIONS(-Wall  -g )

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
SET(CMAKE_INSTALL_PREFIX /opt/local)

ADD_SUBDIRECTORY(boost/coroutine)

INSTALL(FILES boost/coroutine/coroutine.hpp DESTINATION include/boost/coroutine)
INSTALL(FILES 
        boost/coroutine/coasio/acceptor.h 
        boost/coroutine/coasio/addr.h 
        boost/coroutine/coasio/connector.h 
        boost/coroutine/coasio/cosocket.h 
        DESTINATION include/boost/coroutine/coasio)
INSTALL(FILES 
        boost/coroutine/coroutine/coreactor.h 
        boost/coroutine/coroutine/coroutine.h 
        boost/coroutine/coroutine/message.h 
        DESTINATION include/boost/coroutine/coroutine)
INSTALL(FILES 
        boost/coroutine/cothread/cothread.h 
        DESTINATION include/boost/coroutine/cothread)
INSTALL(FILES 
        boost/coroutine/libcoro/coro.h 
        DESTINATION include/boost/coroutine/libcoro)
INSTALL(FILES 
        boost/coroutine/util/linked_allocator.h 
        boost/coroutine/util/list.h 
        boost/coroutine/util/macro.h 
        boost/coroutine/util/none.h 
        boost/coroutine/util/trace.h 
        boost/coroutine/util/var_buf.h 
        DESTINATION include/boost/coroutine/util)
