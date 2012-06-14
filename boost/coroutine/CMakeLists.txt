#the whole CMakeLists.txt for project ttiger
PROJECT(coroutine)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

# include directories
INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}/src
  /opt/local/include
)

# lib directories
LINK_DIRECTORIES(
  ${PROJECT_BINARY_DIR}/lib
  /opt/local/lib
)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
SET(CMAKE_INSTALL_PREFIX /opt/local)

#包含依赖
SET(LIBRARYS pthread ev boost_system)

ADD_LIBRARY(boost_coroutine STATIC 
            libcoro/coro.c 
            coroutine/coroutine.cpp
            coroutine/coreactor.cpp
            cothread/cothread.cpp)
TARGET_LINK_LIBRARIES(boost_coroutine ${LIBRARYS})
SET_TARGET_PROPERTIES(boost_coroutine PROPERTIES VERSION 0.1 SOVERSION 1)

INSTALL(TARGETS boost_coroutine
        ARCHIVE DESTINATION lib)
