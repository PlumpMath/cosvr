/*
 * netcoro.h
 *
 *  Created on: 2012-11-17
 *      Author: yelu
 */

#ifndef CORO_H_
#define CORO_H_

#include <unistd.h>
#include <stdint.h>
#include <context/all.hpp>
#include <coroutine/all.hpp>
#include "log.h"
using boost::context::fcontext_t;

#define DEFAULT_CORO_STACK_SIZE 102400


/**
 * @brief a corontine implenmentation for network io.
 */
class Coroutine
{
public:
	Coroutine(uint32_t stack_size);
	virtual ~Coroutine();

	/*
	 * @brief initialize the ucontext_t structure (data member) to the current
	 * user context of the calling thread. then create a new stack for future use.
	 */
	void declare_as_child(Coroutine& switch_to_when_return,
			    uint32_t stack_size = DEFAULT_CORO_STACK_SIZE);

	void switch_to(Coroutine& to_coro);
	void yield();
	//virtual void callback() = 0;
	virtual void callback()
	{
		LOG_DEBUG("callback.");
	}
private:

	boost::coroutines::detail::stack_allocator _alloc;
	uint32_t _stack_size;
	void* _stack;
	Coroutine* _caller;

	fcontext_t* _context;
};

void coro_callback(intptr_t);

#endif /* CORO_H_ */
