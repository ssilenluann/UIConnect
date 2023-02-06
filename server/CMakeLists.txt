cmake_minimum_required(VERSION 3.01)
project(uiconnect)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -g -O0 -Wall")

target_include_directories(./library/)
target_include_directories(./library/log/)
target_include_directories(./library/log/outputter/)

target_include_directories(./library/network/)
target_include_directories(./library/network/epoll/)
target_include_directories(./library/network/event/)
target_include_directories(./library/network/socket/)

set(LIB_SRC 
	./library/Mutex.cpp
	./library/log/LogFormatter.cpp
	./library/log/Logger.cpp
	./library/log/LoggerManager.cpp
	./library/log/LogItem.cpp
	./library/log/LogLevel.cpp
	./library/log/LogOutputter.cpp
	./library/log/outputter/LogFileOutputter.cpp
	./library/log/outputter/LogStdoutOutputter.cpp
	./library/TcpChannel.cpp
	./library/TcpConnection.cpp
	./library/TcpServer.cpp
	./library/TcpSession.cpp
	./library/epoll/Epoll.cpp
	./library/event/EventLoop.cpp
	./library/event/EventThread.cpp
	./library/event/EventThreadPool.cpp
	./library/socket/Buffer.cpp
	./library/socket/Packet.cpp
	./library/socket/SockAddr.cpp
	./library/socket/Socket.cpp
	./library/socket/TcpSocket.cpp
)

add_executable(server main.cpp ${LIB_SRC})

