/*
 * netcoro.h
 *
 *  Created on: 2012-11-17
 *      Author: yelu
 */

#ifndef EPOLL_CORO_H_
#define EPOLL_CORO_H_

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <errno.h>
#include <map>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "log.h"
#include "Coroutine.h"

class Session;

#define MAX_EVENTS 64

/**
 * @brief a corontine scheduler implenmentation for network io(use epoll).
 */
class Server:public Coroutine
{
public:

	typedef boost::function<void(int, epoll_coro_t*)> NEW_CONN_CALLBACK;

	Server(int listen_port);
	virtual ~Server();

	int Run(NEW_CONN_CALLBACK& new_conn_callback);

	int MonitorReadOnce(int fd, Session* io_coro);

	int MonitorWriteOnce(int fd, Session* io_coro);

	void callback(){}

private:

	static int _CreateAndBind(int port);
	static int _MakeSocketNonBlocking(int sfd);

	void _RunEventLoop();
	int _AcceptNewConn();
	int _HandleNewConn(int);

	int _listen_fd;
	int _listen_port;
	int _epollfd;
	struct epoll_event* _events;	/* Buffer where events are returned */
	std::map<int, Session*> _session_map;
	NEW_CONN_CALLBACK _new_conn_callback;
};

#endif /* EPOLL_CORO_H_ */
