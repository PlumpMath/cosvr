/*
 * Server.cpp
 *
 *  Created on: 2012-11-17
 *      Author: yelu
 */

#include "Server.h"
#include "Session.h"

Server::Server(int listen_port)
{
	_listen_fd = -1;
	_listen_port = listen_port;
	_epollfd = -1;
	_events = new epoll_event[MAX_EVENTS];
}

Server::~Server()
{
	LOG(DEBUG, "enter deconstructor.");
	delete []_events;
	_events = NULL;
	LOG(DEBUG, "delete _events.");
	std::map<int, Session*>::iterator ite = _session_map.begin();
	for (; ite != _session_map.end(); ++ite)
	{
		delete ite->second;
		ite->second = NULL;
	}
	LOG(DEBUG, "delete _session_map.");
}

int Server::MonitorReadOnce(int fd, Session* session)
{
	if(-1 == _epollfd || NULL == session || fd < 0)
	{
		LOG_WARNING("wrong params[fd:%d][session:%p].", fd, session);
		return -1;
	}
	struct epoll_event event;
	event.data.fd = fd;
	// monite read event on socket.
	event.events = EPOLLIN | EPOLLONESHOT;
	if (-1 == epoll_ctl(_epollfd, EPOLL_CTL_MOD, fd, &event))
	{
		LOG_WARNING("epoll_ctl failed[fd:%d][err %d:%s].", _epollfd, errno,
				strerror(errno));
		return -1;
	}
	// add <fd, session*> pair to map.
	_session_map[fd] = session;
	return 0;
}

int Server::MonitorWriteOnce(int fd, Session* session)
{
	if(-1 == _epollfd || NULL == session || fd < 0)
	{
		LOG_WARNING("wrong params[fd:%d][session:%p].", fd, session);
		return -1;
	}
	struct epoll_event event;
	event.data.fd = fd;
	// monite read event on socket.
	event.events = EPOLLOUT | EPOLLONESHOT;
	if (-1 == epoll_ctl(_epollfd, EPOLL_CTL_MOD, fd, &event))
	{
		LOG_WARNING("epoll_ctl failed[fd:%d][err %d:%s].", errno, _epollfd,
				strerror(errno));
		return -1;
	}
	// add <fd, session*> pair to map.
	_session_map[fd] = session;
	return 0;
}

int Server::_CreateAndBind(int port)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;	/* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM;	/* tcp socket */
	hints.ai_flags = AI_PASSIVE;	/* For wildcard IP address */
	hints.ai_protocol = 0;          /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	char port_str[16];
	snprintf(port_str, sizeof(port_str), "%d", port);
	port_str[sizeof(port_str) - 1] = '\0';
	LOG(DEBUG, "will use [port:%s] to bind.", port_str);
	struct addrinfo* result;
	int ret = getaddrinfo(NULL, port_str, &hints, &result);
	if (0 != ret)
	{
		LOG(WARNING, "getaddrinfo error.[gai_strerror:%s].", gai_strerror(ret));
		return -1;
	}

	/* getaddrinfo() returns a list of address structures. Try each address
	 * until we successfully bind(). If socket() (or bind()) fails, we
	 * (close the socket and) try the next address. */
	int sfd = -1;
	int cnt = 0;
	struct addrinfo* rp;
	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		cnt++;
		sfd = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (-1 == sfd)
		{
			LOG(DEBUG, "create socket failed[err %d:%s].", errno, strerror(errno));
			continue;
		}
		// allow port reuse.
		int reuse = 1;
		if (-1 == ::setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse,
				sizeof(reuse)))
		{
			LOG(DEBUG, "setsockopt failed[err %d:%s].", errno, strerror(errno));
			close(sfd);
			continue;
		}
		if (0 == ::bind(sfd, rp->ai_addr, rp->ai_addrlen))
		{
			LOG(DEBUG, "bind successfully!");
			break;
		}
		else
		{
			LOG(DEBUG, "bind err[err %d:%s].", errno, strerror(errno));
			close(sfd);
		}
	}
	LOG(DEBUG, "[addrinfo cnt:%d]", cnt);

	if(NULL == rp)
	{
		LOG(WARNING, "could not bind.");
		return -1;
	}

	freeaddrinfo(result);
	return sfd;
}

int Server::_MakeSocketNonBlocking(int sfd)
{
	int flags = fcntl(sfd, F_GETFL, 0);
	if (-1 == flags)
	{
		LOG(WARNING, "fcntl get flag failed[err %d:%s].", errno, strerror(errno));
		return -1;
	}

	flags |= O_NONBLOCK;
	if (-1 == fcntl(sfd, F_SETFL, flags))
	{
		LOG(WARNING, "fcntl set flag O_NONBLOCK failed[err %d:%s].", errno,
				strerror(errno));
		return -1;
	}
	return 0;
}

int Server::Run(NEW_CONN_CALLBACK& new_conn_callback)
{
	_new_conn_callback = new_conn_callback;
	_listen_fd = _CreateAndBind(_listen_port);
	if (-1 == _listen_fd)
	{
		LOG_WARNING("failed to create and bind.");
		return -1;
	}

	if (0 != _MakeSocketNonBlocking(_listen_fd))
	{
		LOG_WARNING("failed to make_socket_non_blocking.");
		return -1;
	}

	if (0 != ::listen(_listen_fd, SOMAXCONN))
	{
		LOG_WARNING("failed to listen on socket[err %d:%s].", errno,
						strerror(errno));
		return -1;
	}

	_epollfd = ::epoll_create(10);
	if (-1 == _epollfd)
	{
		LOG_WARNING("epoll_create failed[err %d:%s].", errno, strerror(errno));
		return -1;
	}

	struct epoll_event event;
	event.data.fd = _listen_fd;
	// monitor read event on listen socket.
	event.events = EPOLLIN;
	if (-1 == ::epoll_ctl(_epollfd, EPOLL_CTL_ADD, _listen_fd, &event))
	{
		LOG_WARNING("epoll_ctl failed[err %d:%s].", errno, strerror(errno));
		return -1;
	}

	// run event loop.
	_RunEventLoop();
	LOG_DEBUG("event loop ends.");
	return 0;
}

void Server::_RunEventLoop()
{
	for (;;)
	{
		LOG_DEBUG("start epoll_wait.");
		int nfds = ::epoll_wait(_epollfd, _events, MAX_EVENTS, -1);
		LOG_DEBUG("epoll_wait returns[nfds:%d]", nfds);
		if (-1 == nfds)
		{
			LOG_WARNING("epoll_wait error[err %d:%s].", errno,
					strerror(errno));
			return;
		}
		for (int i = 0; i < nfds; i++)
		{
			int fd = _events[i].data.fd;
			if ((_events[i].events & EPOLLERR) || (_events[i].events & EPOLLHUP))
			{
				//TODO:tell session about the err.
				/* An error has occured on this fd, or the socket is not
				 ready for reading (why were we notified then?) */
				LOG_WARNING("epoll error with[fd:%d].", fd);
				close(fd);
				continue;
			}
			else if (_listen_fd == fd)
			{
				/* We have a notification on the listening socket, which
				 means one or more incoming connections. */
				while (true)
				{
					int infd = _AcceptNewConn();
					// nonnegative means an right fd.
					if(infd < 0)
					{
						break;
					}
					// call new conn callback.
					int if_ok = _HandleNewConn(infd);
					if(0 != if_ok)
					{
						LOG_WARNING("failed to accept new conn.");
						::close(infd);
					}
				}
			}
			else
			{
				// a read or write event happens, switch coroutine to handle it.
				Session* session = _session_map[fd];
				LOG_DEBUG("switch coroutine[fd:%d][session:%p]", fd, session);
				switch_to(*session);
			}
		}
	}
}

int Server::_HandleNewConn(int fd)
{
	// make callback to use-given callback. attention exceptions may throw in
	// given callback.
	try
	{
		_new_conn_callback(fd, this);
	}
	catch(std::exception& e)
	{
		LOG_WARNING("exception catched in _handle_new_conn[%s]", e.what());
		return -1;
	}
	catch(...)
	{
		LOG_WARNING("exception catched int _handle_new_conn.");
		return -1;
	}
	return 0;
}

int Server::_AcceptNewConn()
{
	struct sockaddr in_addr;
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	socklen_t in_len = sizeof(in_addr);
	int infd = ::accept(_listen_fd, &in_addr, &in_len);
	if (-1 == infd)
	{
		if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
		{
			// no more incoming connections.
			return -1;
		}
		else
		{
			LOG_WARNING("accept failed[err %d:%s].", errno,
					strerror(errno));
			return -1;
		}
	}

	if (0 == getnameinfo(&in_addr, in_len, hbuf, sizeof(hbuf),
					sbuf, sizeof(sbuf),
					NI_NUMERICHOST | NI_NUMERICSERV))
	{
		LOG_DEBUG("accepted connection on [fd:%d][host:%s]"
				"[port:%s]", infd, hbuf, sbuf);
	}

	/* Make the incoming socket non-blocking and add it to the
	 list of fds to monitor.*/
	if (0 != _MakeSocketNonBlocking(infd))
	{
		LOG_WARNING("make_socket_non_blocking failed. this "
				"connection will be closed and refused.");
		::close(infd);
		return -1;
	}

	struct epoll_event event;
	event.data.fd = infd;
	// only monitor err initially.
	event.events = EPOLLERR | EPOLLHUP;
	if (-1 == epoll_ctl(_epollfd, EPOLL_CTL_ADD, infd, &event))
	{
		LOG_WARNING("epoll_ctl failed[err %d:%s]this connection "
			"will be closed and refused.", errno, strerror(errno));
		close(infd);
		return -1;
	}
	return infd;
}

