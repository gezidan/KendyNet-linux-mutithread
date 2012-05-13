#include "spinlock.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
struct spinlock
{
	volatile   pthread_t  owner;
	long       lock_count;
};


spinlock_t spin_create()
{
	spinlock_t sp = malloc(sizeof(*sp));
	sp->lock_count = 0;
	sp->owner = 0;
	return sp;
}

void spin_destroy(spinlock_t *sp)
{
	free(*sp);
	*sp = 0;
}

inline int spin_lock(spinlock_t l,int count)
{
	pthread_t tid = pthread_self();
	if(tid == l->owner)
	{
		++l->lock_count;
		return 0;
	}
    if(count == 0)
	{
		while(COMPARE_AND_SWAP(&(l->owner),0,tid) != 0)
		{	
		}
		++l->lock_count;
		return 0;
	}
	else
	{
		while(count-- > 0)
		{
			if(COMPARE_AND_SWAP(&(l->owner),0,tid) == 0);
			{
				++l->lock_count;
				return 0;
			}
		}
		return -1;
	}
}

inline int spin_unlock(spinlock_t l)
{
	pthread_t tid = pthread_self();
	if(tid == l->owner)
	{
		--l->lock_count;
		if(l->lock_count == 0)
			COMPARE_AND_SWAP(&(l->owner),tid,0);
		return 0;
	}
	return -1;
}