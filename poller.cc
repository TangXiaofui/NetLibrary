#include "poller.h"

namespace txh{

PollerEpoll::PollerEpoll()
{
	m_fd = epoll_create1(EPOLL_CLOEXEC);
	fatalif(m_fd < 0,"epoll_create failed , %d %s",errno,strerror(errno));
	info("Poller epoll create %d",m_fd);		
}

PollerEpoll::~PollerEpoll()
{
	info("destroying epoll fd %d",m_fd);
	while(m_liveChannels.size())
	{
		(*m_liveChannels.begin())->close();	
	}	
	close(m_fd);
	info("epoll fd %d destroy",m_fd);
}

void PollerEpoll::addChannel(Channel *ch)
{
	struct epoll_event ev;
	memset(&ev,0,sizeof ev);
	ev.events = ch->events();
	ev.data.ptr = ch;
	trace("adding channel %lld fd %d events %d epoll %d",(long long)ch->id(),ch->fd(),ev.events,m_fd);
	int r = epoll_ctl(m_fd,EPOLL_CTL_ADD,ch->fd(),&ev);
	fatalif(r < 0 ,"epoll_ctl add failed %d %s",errno,strerror(errno));
	m_liveChannels.insert(ch);
}

void PollerEpoll::removeChannel(Channel *ch)
{
	trace("deleting channel %lld fd %d epoll %d",(long long)ch->id(),ch->fd(),m_fd);
	m_liveChannels.erase(ch);
	for(int i = m_lastActive; i >= 0; --i)
	{
		if(ch == m_activeEvents[i].data.ptr){
			m_activeEvents[i].data.ptr = NULL;
			break;
		}
	}
}

void PollerEpoll::updateChannel(Channel *ch)
{
	struct epoll_event ev;
	memset(&ev,0,sizeof ev);
	ev.events = ch->events();
	ev.data.ptr = ch;
	trace("modifying channel %lld fd %d events %d epoll %d",(long long)ch->id(),ch->fd(),ev.events,m_fd);
	int r = epoll_ctl(m_fd,EPOLL_CTL_MOD,ch->fd(),&ev);
	fatalif(r < 0, "epoll_ctl mod fail %d %s",errno,strerror(errno));	
}

void PollerEpoll::loop_once(int waitMs)
{
	int64_t ticks = util::timeMilli();
	m_lastActive = epoll_wait(m_fd,m_activeEvents,MaxEvent,waitMs);
	int64_t used = util::timeMilli() - ticks;
	trace("epoll wait %d return %d errno %d used %lld millsecond",waitMs,m_lastActive,errno,(long long)used);
	fatalif(m_lastActive == -1 && errno != EINTR,"epollwait return %d %s",errno,strerror(errno));
	while(--m_lastActive >= 0)
	{
		int i = m_lastActive;
		Channel *ch = (Channel *)m_activeEvents[i].data.ptr;
		int events = ch->events();
		if(ch){
			if(events & (EPOLLERR | ReadEvent )){
				trace("channel %lld fd %d handle read",(long long)ch->id(),ch->fd());
				ch->handleRead();
			}
			else if(events & WriteEvent){
				trace("channel %lld fd %d handle write",(long long)ch->id(),ch->fd());
				ch->handleWrite();
			}
			else{
				fatal("unknow events");
			}
		}
	}
}

PollerBase* createPoller()
{
	return new PollerEpoll();
}


}
