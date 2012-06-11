PROJECT(coroutine)

#包含依赖
SET(LIBRARYS boost_coroutine boost_thread ev)

# include directories
INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}/
  /opt/local/include
)

# lib directories
LINK_DIRECTORIES(
  /opt/local/lib
)

ADD_DEFINITIONS(-Wall)
ADD_DEFINITIONS(-DENABLE_TRACE)

ADD_EXECUTABLE(chatroom chatroom/chatroom.cpp)
TARGET_LINK_LIBRARIES(chatroom ${LIBRARYS})

ADD_EXECUTABLE(chatroom_cothread chatroom/chatroom_cothread.cpp)
TARGET_LINK_LIBRARIES(chatroom_cothread ${LIBRARYS})


