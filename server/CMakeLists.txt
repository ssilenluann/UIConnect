cmake_minimum_required(VERSION 3.01)
project(uiconnect)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -g -O0 -Wall")

# static link boost component
set(Boost_USE_STATIC_LIBS ON)
# specify target link boost component
find_package(Boost REQUIRED COMPONENTS filesystem program_options)
if(NOT Boost_FOUND)
	message(FATAL_ERROR "Could not find required Boost libraries")
endif()

include_directories(${Boost_INCLUDE_DIRS})
include_directories(./external/)
include_directories(./external/)

include_directories(./library/)
include_directories(./library/log/)
include_directories(./library/log/outputter/)
include_directories(./library/thread/)
include_directories(./library/network/)
include_directories(./library/network/epoll/)
include_directories(./library/network/event/)
include_directories(./library/network/socket/)
include_directories(./library/utils)
include_directories(./library/coroutine)
include_directories(./external/yaml-cpp)
set(LIB_SRC 
	./library/Mutex.cpp
	./library/ProgramOpt.cpp
	./library/TimerFunc.cpp
	./library/thread/Thread.cpp
	./library/thread/ThreadPool.hpp
	./library/coroutine/Coroutine.cpp
	./library/coroutine/Scheduler.cpp
	./library/coroutine/IOManager.cpp
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
	./library/utils/StringUtil.cpp
	./library/utils/TimeUtil.cpp
	./library/config/ItemConverter.hpp
	./library/config/ConfigItem.hpp
	./library/config/Config.cpp
	./library/hook/FDItem.cpp
	./library/hook/FDManager.cpp
	./library/hook/Hook.cpp
)
	
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# SET_TARGET_PROPERTIES(dllname PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../bin)

## set dynamic library directory
link_directories(./lib)

# add_executable(test_server ./tests/TestTcpServer.cpp ${LIB_SRC})
# # target_link_options(test_server PUBLIC "-fPIE")
# target_link_libraries(test_server PUBLIC libboost_program_options.a
# libyaml-cpp.so
# libboost_filesystem.a
# )

# add_executable(test_log ./tests/TestLog.cpp ${LIB_SRC})
# target_link_libraries(test_log PUBLIC libboost_program_options.a
# libyaml-cpp.so
# libboost_filesystem.a
# )

add_executable(test_client ./tests/TestTcpClient.cpp ${LIB_SRC})
target_link_libraries(test_client PUBLIC libboost_program_options.a
libyaml-cpp.so
libboost_filesystem.a
)

# add_executable(test_prog_opt ./tests/TestProgOpt.cpp ${LIB_SRC})
# target_link_libraries(test_prog_opt PUBLIC libboost_program_options.a
# libyaml-cpp.so
# libboost_filesystem.a
# )

# add_executable(test_config ./tests/TestConfig.cpp ${LIB_SRC})
# target_link_libraries(test_config PUBLIC libboost_program_options.a
# libyaml-cpp.so
# libboost_filesystem.a
# )
# add_executable(test_thread ./tests/TestThread.cpp ${LIB_SRC})
# target_link_libraries(test_thread PUBLIC libboost_program_options.a
# libyaml-cpp.so
# libboost_filesystem.a
# )

# add_executable(test_coroutine ./tests/TestCoroutine.cpp ${LIB_SRC})
# target_link_libraries(test_coroutine PUBLIC libboost_program_options.a
# libyaml-cpp.so
# libboost_filesystem.a
# )

# add_executable(test_util ./tests/TestUtil.cpp ${LIB_SRC})
# target_link_libraries(test_util PUBLIC libboost_program_options.a
# libyaml-cpp.so
# libboost_filesystem.a
# )

# add_executable(test_scheduler ./tests/TestScheduler.cpp ${LIB_SRC})
# target_link_libraries(test_scheduler PUBLIC ${Boost_LIBRARIES}
# libyaml-cpp.so
# )

# add_executable(test_server ./tests/TestTcpServer.cpp ${LIB_SRC})
# target_link_libraries(test_server yaml-cpp.so)
# target_link_libraries(test_server libboost_program_options.so)

# add_executable(test_log ./tests/TestLog.cpp ${LIB_SRC})
# add_executable(test_client ./tests/TestTcpClient.cpp ${LIB_SRC})

# add_executable(test_epoll_timeout ./tests/TestEpollWait.cpp ${LIB_SRC}) 
# target_link_libraries(test_epoll_timeout PUBLIC libboost_program_options.a
# libyaml-cpp.so
# libboost_filesystem.a
# )

add_executable(test_io_manager ./tests/TestIoManager.cpp ${LIB_SRC})
target_link_libraries(test_io_manager PUBLIC ${Boost_LIBRARIES}
libyaml-cpp.so
)
