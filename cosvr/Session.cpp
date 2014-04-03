/*
 * netcoro.cpp
 *
 *  Created on: 2012-11-17
 *      Author: yelu
 */

#include <errno.h>
#include <unistd.h>
#include <cassert>
#include "iocoro/epoll_coro.h"
#include "iocoro/io_coro.h"
#include "common/log.h"

Session::Session(epoll_coro_t* epoll_coro)
{
	LOG_DEBUG("enter Session constructor.");
	_epoll_coro = epoll_coro;
	assert(NULL != _epoll_coro);
}

Session::~Session()
{
	LOG(DEBUG, "enter deconstructor.");
}

int Session::Run()
{
	// this functio is called in epoll coroutine, switch from epoll coro to
	// io coro to enter io coro callback.
	LOG_DEBUG("[this:%p][epoll_coro_t:%p]", this, _epoll_coro);
	_epoll_coro->switch_to(*this);
}

int Session::Write(fd_t& fd, char* buf, uint64_t len)
{
	if(NULL == buf || len <= 0)
	{
		LOG_WARNING("wrong params.");
		return -1;
	}
	uint64_t cnt = 0;
	int write_fd = fd.get_fd();
	while (cnt < len)
	{
		_epoll_coro->monitor_write_once(write_fd, this);
		yield();
		ssize_t count = ::write(write_fd, buf + cnt, len - cnt);
		LOG_DEBUG("[_epoll_coro:%p][_fd:%d][write:%d]", _epoll_coro, write_fd, count);
		if (count <= 0)
		{
			LOG_WARNING("socket write err[err %d:%s].", errno, strerror(errno));
			return -1;
		}
		cnt += count;
	}
	return 0;
}

int Session::Read(fd_t& fd, char* buf, uint64_t len)
{
	if(NULL == buf || len <= 0)
	{
		LOG_WARNING("wrong params.");
		return -1;
	}
	uint64_t cnt = 0;
	int read_fd = fd.get_fd();
	while (cnt < len)
	{
		_epoll_coro->monitor_read_once(read_fd, this);
		yield();
		ssize_t count = ::read(read_fd, buf + cnt, len - cnt);
		LOG_DEBUG("[_epoll_coro:%p][_fd:%d][read:%d]", _epoll_coro, read_fd, count);
		if (count <= 0)
		{
			LOG_WARNING("socket read err[err %d:%s].", errno, strerror(errno));
			return -1;
		}
		cnt += count;
	}
	return 0;
}







