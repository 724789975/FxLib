/*
 * Copyright (c) 2010-2011, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __HIREDIS_LIBEVENT_H__
#define __HIREDIS_LIBEVENT_H__
#include "event2/event.h"
#include "hiredis.h"
#include "async.h"

typedef struct redisLibeventEvents {
    redisAsyncContext *context;
	struct event* rev;
	struct event* wev;
} redisLibeventEvents;

static void redisLibeventReadEvent(int fd, short event, void *arg) {
    ((void)fd); ((void)event);
    redisLibeventEvents *e = (redisLibeventEvents*)arg;
    redisAsyncHandleRead(e->context);
}

static void redisLibeventWriteEvent(int fd, short event, void *arg) {
    ((void)fd); ((void)event);
    redisLibeventEvents *e = (redisLibeventEvents*)arg;
    redisAsyncHandleWrite(e->context);
}

static void redisLibeventAddRead(void *privdata) {
    redisLibeventEvents *e = (redisLibeventEvents*)privdata;
    event_add(e->rev,NULL);
}

static void redisLibeventDelRead(void *privdata) {
    redisLibeventEvents *e = (redisLibeventEvents*)privdata;
    event_free(e->rev);
	e->rev = NULL;
}

static void redisLibeventAddWrite(void *privdata) {
    redisLibeventEvents *e = (redisLibeventEvents*)privdata;
    event_add(e->wev,NULL);
}

static void redisLibeventDelWrite(void *privdata) {
    redisLibeventEvents *e = (redisLibeventEvents*)privdata;
	event_free(e->wev);
	e->wev = NULL;
}

static void redisLibeventCleanup(void *privdata) {
    redisLibeventEvents *e = (redisLibeventEvents*)privdata;
	event_free(e->rev);
	event_free(e->wev);
    free(e);
}

int redisLibeventAttach(redisAsyncContext *ac, struct event_base *base);

#endif
