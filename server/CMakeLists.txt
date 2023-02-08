cmake_minimum_required(VERSION 3.01)
project(uiconnect)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -g -O0 -Wall")

include_directories(./external/repidjson/include)

include_directories(./library/)
include_directories(./library/log/)
include_directories(./library/log/outputter/)
include_directories(./library/thread/)
include_directories(./library/network/)
include_directories(./library/network/epoll/)
include_directories(./library/network/event/)
include_directories(./library/network/socket/)
include_directories(./library/utils)


set(LIB_SRC 
	./library/Mutex.cpp
	./library/thread/Thread.cpp
	./library/thread/ThreadPool.hpp
	./library/log/LogFormatter.cpp
	./library/log/Logger.cpp
	./library/log/LoggerManager.cpp
	./library/log/LogItem.cpp
	./library/log/LogLevel.cpp
	./library/log/LogOutputter.cpp
	./library/log/outputter/LogFileOutputter.cpp
	./library/log/outputter/LogStdoutOutputter.cpp
	./library/network/TcpChannel.cpp
	./library/network/TcpConnection.cpp
	./library/network/TcpServer.cpp
	./library/network/TcpSession.cpp
	./library/network/epoll/Epoll.cpp
	./library/network/event/EventLoop.cpp
	./library/network/event/EventThread.cpp
	./library/network/event/EventThreadPool.cpp
	./library/network/socket/Buffer.cpp
	./library/network/socket/Packet.cpp
	./library/network/socket/SockAddr.hpp
	./library/network/socket/Socket.hpp
	./library/network/socket/TcpSocket.cpp
	./library/utils/FileSystem.cpp
	./library/utils/ThreadUtil.cpp
)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# SET_TARGET_PROPERTIES(dllname PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../bin)

add_executable(server ./tests/TestTcpServer.cpp ${LIB_SRC})
add_executable(test_log ./tests/TestLog.cpp ${LIB_SRC})
add_executable(test_client ./tests/TestTcpClient.cpp ${LIB_SRC})
