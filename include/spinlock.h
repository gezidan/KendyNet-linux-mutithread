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
#ifndef _SPINLOCK_H
#define _SPINLOCK_H
#include "atomic.h"

typedef struct spinlock *spinlock_t;



spinlock_t spin_create();
void spin_destroy(spinlock_t*);

/*
* 锁定返回0
* count:自旋的次数,0:一直尝试,直到成功锁定
*/
inline int spin_lock(spinlock_t,int count);
inline int spin_unlock(spinlock_t);

static inline int
atomic_cmp_set(int *lock, int old,
    int set)
{
    char  res;
	//"    cmpxchgq  %3, %1;   "
    __asm__ volatile (

    "lock;"
    "    cmpxchgl  %3, %1;   "
    "    sete      %0;       "

    : "=a" (res) : "m" (*lock), "a" (old), "r" (set) : "cc", "memory");

    return res;
}

#endif