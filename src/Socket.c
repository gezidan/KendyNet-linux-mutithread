#include "Socket.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include "SocketWrapper.h"
#include "KendyNet.h"

socket_t create_socket()
{
	socket_t s = malloc(sizeof(*s));
	if(s)
	{
		s->mtx = mutex_create();
		s->recv_mtx = mutex_create();
		s->send_mtx = mutex_create();
		s->pending_send = LIST_CREATE();
		s->pending_recv = LIST_CREATE();
		s->status = 0;
		s->engine = 0;
	}
	return s;
}

void free_socket(socket_t *s)
{
	assert(s);assert(*s);
	mutex_destroy(&(*s)->mtx);
	mutex_destroy(&(*s)->recv_mtx);
	mutex_destroy(&(*s)->send_mtx);
	destroy_list(&(*s)->pending_send);
	destroy_list(&(*s)->pending_recv);
	free(*s);
	*s = 0;
}

void on_read_active(socket_t s)
{
	assert(s);
	mutex_lock(s->recv_mtx);
	s->readable = 1;
	int active_recv_count = ++s->active_read_count;
	st_io *req = LIST_POP(st_io*,s->pending_recv);
	mutex_unlock(s->recv_mtx);
	
	if(req)
	{
		if(s->engine->mode == MODE_COMPLETE)
			_recv(s,req,active_recv_count,1);
		else
			put_event(s->engine,req);
	}
}

void on_write_active(socket_t s)
{
	assert(s);
	mutex_lock(s->send_mtx);
	s->writeable = 1;
	int active_send_count = ++s->active_write_count;
	st_io *req = LIST_POP(st_io*,s->pending_send);
	mutex_unlock(s->send_mtx);
	if(req)
	{
		if(s->engine->mode == MODE_COMPLETE)
			_send(s,req,active_send_count,1);
		else
			put_event(s->engine,req);
	}		
}

int _recv(socket_t s,st_io* io_req,int active_recv_count,int notify)
{
	assert(s);assert(io_req);
	while(1)
	{
		int retry = 0;
		int bytes_transfer = io_req->bytes_transfer = TEMP_FAILURE_RETRY(readv(s->fd,io_req->iovec,io_req->iovec_count));
		io_req->error_code = 0;
		if(bytes_transfer < 0)
		{
			switch(errno)
			{
				case EAGAIN:
				{
					mutex_lock(s->recv_mtx);
					if(active_recv_count != s->active_read_count)
					{
						active_recv_count = s->active_read_count;
						retry = 1;
					}
					else
					{
						s->readable = 0;
						LIST_PUSH_FRONT(s->pending_recv,io_req);
					}
					mutex_unlock(s->recv_mtx);
					
					if(retry)
						continue;
					return 0;
				}
				break;
				default://连接出错
				{
					io_req->error_code = errno;
				}
				break;
			}
		}
		else if(bytes_transfer == 0)
			bytes_transfer = -1;
		if(notify)
			put_event(s->engine,io_req);	
		return bytes_transfer;	
	}	
}

int _send(socket_t s,st_io* io_req,int active_send_count,int notify)
{
	assert(s);assert(io_req);
	while(1)
	{
		int retry = 0;
		int bytes_transfer = io_req->bytes_transfer = TEMP_FAILURE_RETRY(writev(s->fd,io_req->iovec,io_req->iovec_count));
		io_req->error_code = 0;	
		if(bytes_transfer < 0)
		{
			switch(errno)
			{
				case EAGAIN:
				{
					mutex_lock(s->send_mtx);
					if(active_send_count != s->active_write_count)
					{
						active_send_count = s->active_write_count;
						retry = 1;
					}
					else
					{
						s->writeable = 0;
						//将请求重新放回到队列
						LIST_PUSH_FRONT(s->pending_send,io_req);
					}
					mutex_unlock(s->send_mtx);
					
					if(retry)
						continue;
					return 0;
				}
				break;
				default://连接出错
				{
					io_req->error_code = errno;
				}
				break;
			}
		}
		else if(bytes_transfer == 0)
			bytes_transfer = -1;
		if(notify)
			put_event(s->engine,io_req);	
		return bytes_transfer;	
	}
}