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
#ifndef _TIMING_WHEEL_H
#define _TIMING_WHEEL_H

//��ʱ����
typedef struct WheelItem   *WheelItem_t;
typedef struct TimingWheel *TimingWheel_t;
typedef void   (*TimingWheel_callback)(void*);

/*
* precision:��С����,�Ժ���Ϊ��λ
* max:����ܶ�����ʱ��,�Ժ���Ϊ��λ
*/
extern TimingWheel_t CreateTimingWheel(unsigned long precision,unsigned long max);
extern void DestroyTimingWheel(TimingWheel_t*);
extern int  RegisterTimer(TimingWheel_t,WheelItem_t,unsigned long timeout);
extern void  UnRegisterTimer(TimingWheel_t,WheelItem_t);
extern int  UpdateWheel(TimingWheel_t,unsigned long now);
extern WheelItem_t CreateWheelItem(void *ud,TimingWheel_callback callback);
extern void DestroyWheelItem(WheelItem_t*);
#endif