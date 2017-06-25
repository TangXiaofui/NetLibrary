#ifndef EVENTBASE_H
#define EVENTBASE_H
#include <memory>
#include <algorithm>
#include <atomic>
#include <map>
#include <unistd.h>
#include <fcntl.h>
#include <utility>
#include "logging.h"
#include "channel.h"
#include "poller.h"
#include "util.h"
#include "thread.h"

namespace txh{
using namespace std;

typedef pair<int64_t,int64_t> TimerId;
struct TimerRepeatable{
	int64_t at;
	int64_t interval;
	TimerId tid;
	Task task;	
};

struct Channel;
struct PollerBase;

//下次框架和具体实现要分开，这样框架代码简洁，而实现可以复杂
struct EventBase:public noncopyable{
	EventBase(int taskCapacity = 0);
	~EventBase();
	
	void loop_once(int waitMs);
	void loop();
	
	bool cancel(TimerId timerid);

	TimerId runAt(int64_t waitUs,Task &&task,int64_t interval = 0);
	TimerId runAt(int64_t waitUs,const Task &&task,int64_t interval = 0){
		return runAt(waitUs,Task(task),interval);
	}
	TimerId runAfter(int64_t waitUs,Task &&task,int64_t interval = 0){
		return runAt(util::timeMilli()+waitUs,Task(task),interval);
	}
	TimerId runAfter(int64_t waitUs,const Task &&task,int64_t interval = 0){
		return runAfter(waitUs,Task(task),interval);
	}

	EventBase& exit();
	bool exited();
	void wakeup();
	void safeCall(Task&& task);
	void safeCall(const Task &&task) { safeCall(Task(task)); };
	void handleTimeout();
	void repeatableTimeout(TimerRepeatable *tr);
	void refreshNearest(const TimerId *tid = NULL);
	

	virtual EventBase* allocBase(){ return this; }

	
	PollerBase *m_poller;
	atomic<bool> m_exit;
	SafeQueue<Task> m_tasks;
	int m_wakeupfd[2];
	int m_nextTimeout;
	atomic<int64_t> m_timerNum;
	map<TimerId,Task> m_timers;	
	map<TimerId,TimerRepeatable> m_timerRepeats;	
};

}
#endif
