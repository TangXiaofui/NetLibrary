#include "channel.h"

namespace txh{

Channel::Channel(EventBase *base,int fd , int events)
:m_base(base),m_fd(fd),m_events(events)
{
	fatalif(net::setNonBlock(fd)< 0, "channel set no block failed");
	static atomic<int64_t> id(0);
	m_id = ++id;
	m_poller = m_base->m_poller;
	m_poller->addChannel(this);
}

void Channel::close()
{
	if(m_fd > 0)
	{
		info("close channel id:%lld fd: %d",(long long)m_id,m_fd);
		m_poller->removeChannel(this);
		::close(m_fd);
		m_fd = -1;
		handleRead();
	}
}

void Channel::enableRead(bool enable)
{
	if(enable)
		m_events |= ReadEvent;
	else
		m_events &= ~ReadEvent;
	m_poller->updateChannel(this);
}

void Channel::enableWrite(bool enable)
{
	if(enable)
		m_events |= WriteEvent;
	else
		m_events &= ~WriteEvent;
	m_poller->updateChannel(this);

}

void Channel::enableReadWrite(bool readEnable,bool writeEnable)
{
	if(readEnable)
		m_events |= ReadEvent;
	else
		m_events &= ~ReadEvent;
	if(writeEnable)
		m_events |= WriteEvent;
	else
		m_events &= ~WriteEvent;
	m_poller->updateChannel(this);
}

bool Channel::readEnable()
{
	return m_events & ReadEvent;
}

bool Channel::writeEnable()
{
	return m_events & WriteEvent;
}
Channel::~Channel()
{
	close();	
}
	
}
