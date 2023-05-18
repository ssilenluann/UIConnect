#ifndef NETWORK_TCPSERVER_CPP
#define NETWORK_TCPSERVER_CPP

#include <iostream>
#include "TcpServer.h"
#include "../log/Logger.h"
static Logger::ptr g_logger = LOG_NAME("system");

TcpServer::TcpServer(int threadCount)
: m_acceptor(new TcpAcceptor(threadCount)), m_isStoped(false)
{
}

TcpServer::~TcpServer(){}

void TcpServer::run(const std::string& addr)
{
	LOG_INFO(g_logger) << "server started";
	m_acceptor->run(addr);
	LOG_INFO(g_logger) << "server main loop ended";
}

void TcpServer::exit()
{
	LOG_INFO(g_logger) << "server start quit";
	m_acceptor->exit();
	if(m_quitCallback)	m_quitCallback();
	LOG_INFO(g_logger) << "server quited";
}

void TcpServer::setStartCallback(CALLBACK func)
{
	m_startCallback = func;
}

void TcpServer::setErrorCallback(CALLBACK func)
{
	m_acceptor->setErrorCallback(func);
}

void TcpServer::setQuitCallback(CALLBACK func)
{
	m_quitCallback = func;
}
#endif
