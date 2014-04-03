/*
 * netcoro.cpp
 *
 *  Created on: 2012-11-17
 *      Author: yelu
 */

#include <exception>
#include "Coro.h"
#include "log.h"
#include <signal.h>
#include <context/stack_allocator.hpp>

Coroutine::Coroutine()
{
	_stack = _alloc.allocate(stack_size);
	_stack_size = stack_size;
	_context = boost::context::make_fcontext(_stack, _stack_size, coro_callback);
	LOG_DEBUG("[stack size:%d]", _stack_size);

		// Swapping stacks makes tools like Valgrind get pretty crazy. Lwan��s
		// implementation uses Valgrind-provided macros that marks the newly-allocated
		// blocks (from the heap) as stacks.
	#ifdef USE_VALGRIND
		coro->vg_stack_id = VALGRIND_STACK_REGISTER(this->_stack, this->_stack +
				                                    _stack_size);
	#endif
}

Coroutine::~Coroutine()
{
	LOG(DEBUG, "enter deconstructor.");
	_alloc.deallocate(_stack, _stack_size);
	LOG(DEBUG, "_stack deleted.");
}

void coro_callback(intptr_t p)
{
	LOG_DEBUG("enter callback.");
	Coroutine* pcoro = (Coroutine*)p;
	LOG_DEBUG("pcoro=%p", pcoro);
	pcoro->callback();
	LOG_DEBUG("callback end.");
}

void Coroutine::declare_as_child(Coroutine& switch_to_when_return,
		uint32_t stack_size)
{
	_stack = _alloc.allocate(stack_size);
	_stack_size = stack_size;
	_context = boost::context::make_fcontext(_stack, _stack_size, coro_callback);
	LOG_DEBUG("[stack size:%d]", _stack_size);

	// Swapping stacks makes tools like Valgrind get pretty crazy. Lwan��s
	// implementation uses Valgrind-provided macros that marks the newly-allocated
	// blocks (from the heap) as stacks.
#ifdef USE_VALGRIND
	coro->vg_stack_id = VALGRIND_STACK_REGISTER(this->_stack, this->_stack +
			                                    _stack_size);
#endif
}

void Coroutine::switch_to(Coroutine& to_coro)
{
	to_coro._caller = this;
	LOG_DEBUG("switch coroutine[from:%p][to:%p].", this, &to_coro);
	boost::context::jump_fcontext(_context, to_coro._context, 0);
}

void Coroutine::yield()
{
	boost::context::jump_fcontext(this->_context, _caller->_context, 0);
}



