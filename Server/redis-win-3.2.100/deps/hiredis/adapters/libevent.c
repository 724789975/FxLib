#include "libevent.h"
#include "event2/event_compat.h"

int redisLibeventAttach(redisAsyncContext *ac, struct event_base *base) {
    redisContext *c = &(ac->c);
    redisLibeventEvents *e;

    /* Nothing should be attached when something is already attached */
    if (ac->ev.data != NULL)
        return REDIS_ERR;

    /* Create container for context and r/w events */
    e = (redisLibeventEvents*)malloc(sizeof(*e));
    e->context = ac;

    /* Register functions to start/stop listening for events */
    ac->ev.addRead = redisLibeventAddRead;
    ac->ev.delRead = redisLibeventDelRead;
    ac->ev.addWrite = redisLibeventAddWrite;
    ac->ev.delWrite = redisLibeventDelWrite;
    ac->ev.cleanup = redisLibeventCleanup;
    ac->ev.data = e;

    /* Initialize and install read/write events */
	e->rev = event_new(NULL, INVALID_SOCKET, 0, NULL, NULL);
	e->wev = event_new(NULL, INVALID_SOCKET, 0, NULL, NULL);
    event_set(e->rev,c->fd,EV_READ,redisLibeventReadEvent,e);
    event_set(e->wev,c->fd,EV_WRITE,redisLibeventWriteEvent,e);
    event_base_set(base,e->rev);
    event_base_set(base,e->wev);
    return REDIS_OK;
}

