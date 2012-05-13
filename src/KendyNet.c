#include "KendyNet.h"
#include "Engine.h"
#include "Socket.h"
#include "link_list.h"
#include "HandleMgr.h"
#include <assert.h>
#include "MsgQueue.h"

int InitNetSystem()
{
	return InitHandleMgr();
}

struct block_queue* CreateEventQ()
{
	return BLOCK_QUEUE_CREATE();
}

void DestroyEventQ(struct block_queue** eq)
{
	BLOCK_QUEUE_DESTROY(eq);
}

int EngineRun(HANDLE engine)
{
	engine_t e = GetEngineByHandle(engine);
	if(!e)
		return -1;
	e->Loop(e);	
	return 0;
}

HANDLE CreateEngine(char mode)
{
	HANDLE engine = NewEngine();
	if(engine >= 0)
	{
		engine_t e = GetEngineByHandle(engine);
		e->mode = mode;
		LIST_CLEAR(e->buffering_event_queue);
		LIST_CLEAR(e->block_thread_queue);
		if(0 != e->Init(e))
		{
			CloseEngine(engine);
			engine = -1;
		}
	}
	return engine;
}

void CloseEngine(HANDLE handle)
{
	ReleaseEngine(handle);
}

int Bind2Engine(HANDLE e,HANDLE s)
{
	engine_t engine = GetEngineByHandle(e);
	socket_t sock   = GetSocketByHandle(s);
	if(!engine || ! sock)
		return -1;
	if(engine->Register(engine,sock) == 0)
	{
		sock->engine = engine;
		return 0;
	}
	return -1;
}

//int	GetQueueEvent(HANDLE handle, struct block_queue *EventQ,st_io **io ,int timeout)
int	GetQueueEvent(HANDLE handle, MsgQueue_t EventQ,st_io **io ,int timeout)
{
	assert(EventQ);assert(io);
/*	
	engine_t e = GetEngineByHandle(handle);
	if(!e)
		return -1;
	spin_lock(e->mtx,0);//mutex_lock(e->mtx);
	if(e->status == 0)
	{
		spin_unlock(e->mtx);//mutex_unlock(e->mtx);
		return -1;
	}
	
	if(*io = LIST_POP(st_io*,e->buffering_event_queue))
	{
		spin_unlock(e->mtx);//mutex_unlock(e->mtx);
		return 0;
	}
	//插入到等待队列中，阻塞
	LIST_PUSH_FRONT(e->block_thread_queue,EventQ);
	spin_unlock(e->mtx);//mutex_unlock(e->mtx);

	if(POP_FORCE_WAKE_UP == BLOCK_QUEUE_POP(EventQ,io,timeout))
		return -1;//因为engine的释放而被强制唤醒
	*/
	engine_t e = GetEngineByHandle(handle);
	if(!e)
		return -1;
	mutex_lock(e->mtx);
	if(e->status == 0)
	{
		mutex_unlock(e->mtx);
		return -1;
	}
	
	if(*io = LIST_POP(st_io*,e->buffering_event_queue))
	{
		mutex_unlock(e->mtx);
		return 0;
	}
	//插入到等待队列中，阻塞
	LIST_PUSH_FRONT(e->block_thread_queue,EventQ);
	mutex_unlock(e->mtx);

	GetMsg(EventQ,io,sizeof(*io),0);
	if(*io == 0)
		return -1;
	
	return 0;	
}

int	PutQueueEvent(HANDLE handle,st_io *io)
{
	assert(io);
	engine_t e = GetEngineByHandle(handle);
	if(!e)
		return -1;
	
	return put_event(e,io);
}

extern int put_event(engine_t e,st_io *io);

int WSASend(HANDLE sock,st_io *io,int notify)
{
	assert(io);
	socket_t s = GetSocketByHandle(sock);
	if(!s)
		return -1;
	
	int active_send_count = -1;
	int ret = 0;
	
	mutex_lock(s->send_mtx);
	//为保证执行顺序与请求的顺序一致,先将请求插入队列尾部,再弹出队列首元素
	LIST_PUSH_BACK(s->pending_send,io);
	io = 0;
	if(s->writeable)
	{
		io = LIST_POP(st_io*,s->pending_send);
		active_send_count = s->active_write_count;
	}
	mutex_unlock(s->send_mtx);
	/*
	if(io)
	{
		if(s->engine->mode == MODE_POLL)
		{
			if(notify)
				put_event(s->engine,io);
			else
				ret =  _send(s,io,active_send_count,notify);

		}
		else
			ret =  _send(s,io,active_send_count,notify);
	}
	*/
	if(io)
	{
		if(s->engine->mode == MODE_POLL && notify)
		{
			//不直接处理,将处理交给完成线程 
			put_event(s->engine,io);
			return 0;
		}	
		ret =  _send(s,io,active_send_count,notify);
	}
	return ret;
}

int WSARecv(HANDLE sock,st_io *io,int notify)
{
	assert(io);
	socket_t s = GetSocketByHandle(sock);
	if(!s)
		return -1;

	int active_recv_count = -1;
	int ret = 0;
	
	mutex_lock(s->recv_mtx);
	//为保证执行顺序与请求的顺序一致,先将请求插入队列尾部,再弹出队列首元素
	LIST_PUSH_BACK(s->pending_recv,io);
	io  = 0;
	if(s->readable)
	{
		io = LIST_POP(st_io*,s->pending_recv);
		active_recv_count = s->active_read_count;
	}
	mutex_unlock(s->recv_mtx);
	
	if(io)
	{
		if(s->engine->mode == MODE_POLL && notify)
		{
			//不直接处理,将处理交给完成线程 
			put_event(s->engine,io);
			return 0;
		}	
		ret =  _recv(s,io,active_recv_count,notify);
	}
	/*
	if(io)
	{
		if(s->engine->mode == MODE_POLL)
		{
			if(notify)
				put_event(s->engine,io);
			else
				ret =  _recv(s,io,active_recv_count,notify);

		}
		else
			ret =  _recv(s,io,active_recv_count,notify);
	}
	*/
	return ret;
}





