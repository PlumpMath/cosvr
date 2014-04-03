/*
 * test_netcoro.cpp
 *
 *  Created on: 2012-11-17
 *      Author: yelu
 */

/* testcontext.c : demo of ucontex_t operations */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <iocoro/coro.h>
#include "common/log.h"

int main(void)
{
    LOG(DEBUG, "start");
    coro_t main_coro;
    const int CORO_ARR_SIZE = 10000;
    coro_t* coro_arr[CORO_ARR_SIZE];
    for(int i = 0; i < CORO_ARR_SIZE; i++)
    {
    	coro_arr[i] = new coro_t();
    	coro_arr[i]->declare_as_child(main_coro);
    }

    struct timeval tv_begin, tv_end;
    gettimeofday(&tv_begin, NULL);
    for(int i = 0; i < CORO_ARR_SIZE; i++)
    {
    	main_coro.switch_to(*coro_arr[i]);
    }
    gettimeofday(&tv_end, NULL);
	int time = 1000 * (tv_end.tv_sec - tv_begin.tv_sec) + (tv_end.tv_usec - tv_begin.tv_usec)/1000;
	LOG(DEBUG, "time:%d ms", time);
	gettimeofday(&tv_begin, NULL);
	for(int i = 0; i < CORO_ARR_SIZE; i++)
	{
		coro_callback((intptr_t)coro_arr[i]);
	}
	gettimeofday(&tv_end, NULL);
	time = 1000 * (tv_end.tv_sec - tv_begin.tv_sec) + (tv_end.tv_usec - tv_begin.tv_usec)/1000;
	LOG(DEBUG, "time:%d ms", time);


    return 0;
}
