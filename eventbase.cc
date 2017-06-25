#include "eventbase.h"

namespace txh{
	
EventBase::EventBase(int taskCapacity)
:m_poller(createPoller())
,m_tasks(taskCapacity)
,m_exit(false)
,m_nextTimeout(1<<30)
,m_timerNum(0)
{
	int r = pipe(m_wakeupfd);
	fatalif(r < 0 , "pipe fail %d %s",errno,strerror(errno));
	r = util::addFdFlag(m_wakeupfd[0],O_CLOEXEC);
	fatalif(r < 0, "addFdFlag fail %d %s",errno,strerror(errno));
	r = util::addFdFlag(m_wakeupfd[1],O_CLOEXEC);
	fatalif(r < 0, "addFdFlag fail %d %s",errno,strerror(errno));

	info("wakeup pipe created %d %d",m_wakeupfd[0],m_wakeupfd[1]);
	Channel *ch = new Channel(this,m_wakeupfd[0],ReadEvent);
	ch->onRead([=]{
		char buf[1024];
		int r = ch->fd() > 0 ? ::read(ch->fd(),buf,sizeof(buf)) : 0 ;
		if(r > 0){
			Task task;
			while(m_tasks.pop_wait(&task,0)){
				task();
			}
		}
		else if(r == 0)	
			delete ch;
		else if(errno == EINTR){
		}
		else
			fatal("wakeupfd channel read error %d %s",errno,strerror(errno));
	});
}

EventBase::~EventBase()
{
	delete m_poller;
	::close(m_wakeupfd[1]);
}

void EventBase::handleTimeout()
{
	int64_t now = util::timeMilli();
	TimerId tid{now , 1L << 62};
	while(m_timers.size() && m_timers.begin()->first < tid){
		//task 前面不能用&&
		Task task = move(m_timers.begin()->second);
		m_timers.erase(m_timers.begin());
		task();
	}
	refreshNearest();
}


void EventBase::refreshNearest(const TimerId *tid)
{
	if(m_timers.empty())
		m_nextTimeout = 1 << 30;
	else{
		const TimerId &t = m_timers.begin()->first;
		m_nextTimeout = t.first - util::timeMilli();
		m_nextTimeout = m_nextTimeout < 0 ? 0 : m_nextTimeout; 
	}		
}

void EventBase::loop_once(int waitUs)
{
	m_poller->loop_once(min(waitUs,m_nextTimeout));
	handleTimeout();
}


void EventBase::loop()
{
	while(!m_exit){
		loop_once(1000);	
	}
	m_timers.clear();
	m_timerRepeats.clear();
	//Tcp重连
	//
	//
	loop_once(0);
}

bool EventBase::cancel(TimerId timerid)
{
	if(timerid.first < 0){
		auto p = m_timerRepeats.find(timerid);
		auto ptimer = m_timers.find(p->second.tid);
		if(ptimer != m_timers.end())
			m_timers.erase(ptimer);
		m_timerRepeats.erase(p);
		return true;	
	}
	else{
		auto p = m_timers.find(timerid);
		if(p != m_timers.end()){
			m_timers.erase(p);
			return true;
		}
		else
			return false;
	}
}


void EventBase::repeatableTimeout(TimerRepeatable *tr)
{
	tr->at += tr->interval;
	tr->tid = {tr->at,++m_timerNum};
	m_timers[tr->tid] = [this,tr]{ repeatableTimeout(tr); };
	refreshNearest();
	tr->task();	
}


TimerId EventBase::runAt(int64_t waitUs,Task &&task,int64_t interval)
{
	if(m_exit)
		return TimerId();
	if(interval){
		TimerId tid = {-waitUs,++m_timerNum};
		TimerRepeatable &tr = m_timerRepeats[tid];
		tr = {waitUs,interval,{waitUs,++m_timerNum},move(task)};
		TimerRepeatable *p = &tr;
		m_timers[p->tid] = [this,p]{ repeatableTimeout(p); };
		refreshNearest();
		return tid;
	}
	else{
		TimerId tid = {waitUs,++m_timerNum};
		m_timers.insert({tid,move(task)});
		refreshNearest(&tid);
		return tid;
	}
		
}

EventBase& EventBase::exit()
{
	m_exit.store(true);
	wakeup();
	return *this;
}

bool EventBase::exited()
{
	return m_exit;
}

void EventBase::wakeup()
{
	int r = ::write(m_wakeupfd[1],"",1);
	fatalif(r <= 0 , "write m_wakeupfd fail %d %s",errno,strerror(errno));
}

void EventBase::safeCall(Task&& task)
{
	m_tasks.push(move(task));
	wakeup();
}

}
