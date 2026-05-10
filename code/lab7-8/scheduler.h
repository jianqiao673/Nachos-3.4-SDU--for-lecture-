// scheduler.h 
//	Data structures for the thread dispatcher and scheduler.
//	Primarily, the list of threads that are ready to run.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "copyright.h"
#include "list.h"
#include "thread.h"

// The following class defines the scheduler/dispatcher abstraction -- 
// the data structures and operations needed to keep track of which 
// thread is running, and which threads are ready but not running.

class Scheduler {
  public:
    Scheduler();			// Initialize list of ready threads 
    ~Scheduler();			// De-allocate ready list

    void ReadyToRun(Thread* thread);	// Thread can be dispatched.
    Thread* FindNextToRun();		// Dequeue first thread on the ready 
					// list, if any, and return thread.
    void Run(Thread* nextThread);	// Cause nextThread to start running
    void Print();			// Print contents of ready list
    List* getReadyList(); //返回调度器的就绪队列
    List* getWaitingList(); //返回调度器的等待队列
    List* getTerminatedList(); //返回调度器的终止队列
    void deleteTerminatedThread(int SpaceId); //删除终止队列中地址空间ID为SpaceId的线程
    void emptyList(List *list); //清空进程队列
  private:
    List *readyList;  		// queue of threads that are ready to run,
		List *waitingList;    //等待线程队列，线程调用Join()后进入该队列，直到被等待的线程退出
    List *terminatedList; //线程调用Finish()后进入该队列，Joiner
};

#endif // SCHEDULER_H
