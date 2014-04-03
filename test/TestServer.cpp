/*
 *  TestServer.cpp
 *
 *  Created on: 2012-11-17
 *      Author: yelu
 */

/* testcontext.c : demo of ucontex_t operations */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "Server.h"
#include "Session.h"
#include "log.h"

class session_t: public io_coro_t
{
public:
	session_t(epoll_coro_t* epoll):io_coro_t(epoll){}
	void callback()
	{
		char buf[256];
		read(client_fd, buf, 7);
		printf("%s\n", buf);
		write(client_fd, buf, 7);
		client_fd.close();
	}
	static void make_new_session(int fd, epoll_coro_t* epoll_coro)
	{
		LOG_DEBUG("[fd:%d][epoll_coro_t:%p]", fd, epoll_coro);
		session_t* s = new session_t(epoll_coro);
		ss[0] = s;
		s->client_fd.reset_fd(fd);
		s->declare_as_child(*epoll_coro);
		s->run();
	}
private:
	static session_t* ss[256];
	fd_t client_fd;
};
session_t* session_t::ss[256];

int main(void)
{
    LOG_DEBUG("start");

    Server server(20003);
    //epoll_coro.declare_as_main();
    try
    {
    	Server::NEW_CONN_CALLBACK callback(session_t::make_new_session);
    	server.Run(callback);
    }
    catch(std::exception& e)
    {
    	LOG_WARNING("%s", e.what());
    	return 1;
    }
    return 0;
}
