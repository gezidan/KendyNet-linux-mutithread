/*	
    Copyright (C) <2012>  <huangweilook@21cn.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/	
#ifndef _KENDYNET_H
#define _KENDYNET_H
#include "MsgQueue.h"
typedef struct list_node
{
	struct list_node *next;
}list_node;

#define LIST_NODE list_node node;

/*IO请求和完成队列使用的结构*/
typedef struct
{
	LIST_NODE;
	struct iovec *iovec;
	int    iovec_count;
	int    bytes_transfer;
	int    error_code;
}st_io;

enum
{	
	//完成模式,当套接口从不可读/写变为激活态时, 如果有IO请求,则完成请求,并将完成通告发送给工作线程
	MODE_COMPLETE = 0,
	//poll模式,当套接口从不可读/写变为激活态时, 如果有IO请求,将请求发送给工作线程,由工作线程完成请求
	MODE_POLL,
};

//初始化网络系统
int      InitNetSystem();

typedef int HANDLE;
struct block_queue;

struct block_queue* CreateEventQ();
void DestroyEventQ(struct block_queue**);

HANDLE   CreateEngine(char);
void     CloseEngine(HANDLE);
int      EngineRun(HANDLE);

int     Bind2Engine(HANDLE,HANDLE);

//获取和投递事件到engine的队列中
//int      GetQueueEvent(HANDLE,struct block_queue*,st_io **,int timeout);
int      GetQueueEvent(HANDLE,MsgQueue_t,st_io **,int timeout);
int      PutQueueEvent(HANDLE,st_io *);

/*
*   发送和接收,如果操作立即完成返回完成的字节数
*   否则返回0,当IO异步完成后会通告到事件队列中.
*   返回-1表示套接口出错或断开
*   notify:当IO立即可完成时,是否向事件队
*   列提交一个完成通告,设置为1会提交,0为不提交.
*/
int WSASend(HANDLE,st_io*,int notify);
int WSARecv(HANDLE,st_io*,int notify);

#endif