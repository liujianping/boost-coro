#the whole CMakeLists.txt for project ttiger
PROJECT(coroutine)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

# include directories
INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}/
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/libcoro
  ${PROJECT_SOURCE_DIR}/libev
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

ADD_SUBDIRECTORY(src)
#ADD_SUBDIRECTORY(examples)

INSTALL(FILES src/coroutine.hpp DESTINATION include/boost/coroutine)
INSTALL(FILES 
        src/coasio/acceptor.h 
        src/coasio/addr.h 
        src/coasio/connector.h 
        src/coasio/cosocket.h 
        DESTINATION include/boost/coroutine/coasio)
INSTALL(FILES 
        src/coroutine/coreactor.h 
        src/coroutine/coroutine.h 
        src/coroutine/message.h 
        DESTINATION include/boost/coroutine/coroutine)
INSTALL(FILES 
        src/cothread/cothread.h 
        DESTINATION include/boost/coroutine/cothread)
INSTALL(FILES 
        src/libcoro/coro.h 
        DESTINATION include/boost/coroutine/libcoro)
INSTALL(FILES 
        src/util/linked_allocator.h 
        src/util/list.h 
        src/util/macro.h 
        src/util/none.h 
        src/util/trace.h 
        src/util/var_buf.h 
        DESTINATION include/boost/coroutine/util)
