#include "net.h"
namespace txh{

int net::setNonBlock(int fd,bool value)
{
	int flags = fcntl(fd,F_GETFL,0);
	if(flags < 0)
		return errno;
	
	if(value)
		return fcntl(fd,F_SETFL,flags | O_NONBLOCK);
	return fcntl(fd,F_SETFL,flags & ~ O_NONBLOCK);	
}

int net::setReuseAddr(int fd, bool value)
{
	int flags = value;
	int len = sizeof(flags);
	return setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&flags,len);		
}

int net::setReusePort(int fd,bool value)
{
	int flags = value;
	int len = sizeof(flags);
	return setsockopt(fd,SOL_SOCKET,SO_REUSEPORT,&flags,len);	
}

int net::setNoDelay(int fd, bool value)
{
	int flags = value;
	int len = sizeof flags;
	return setsockopt(fd,SOL_SOCKET,TCP_NODELAY,&flags,len);	
}


Ip4Addr::Ip4Addr(const string &host,short port)
{
	memset(&m_addr,0,sizeof m_addr);
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(port);
	if(host.size())
		m_addr.sin_addr =  port::getHostByName(host);
	else
		m_addr.sin_addr.s_addr = INADDR_ANY;
	
	if(m_addr.sin_addr.s_addr == INADDR_NONE)
		error("cann't resove %s to ip ",host.c_str());	
}

string Ip4Addr::toString() const
{
	return util::format("%s:%d",ip().c_str(),port());
}

string Ip4Addr::ip() const
{
	uint32_t uip = m_addr.sin_addr.s_addr;
	return util::format("%d.%d.%d.%d",
				(uip >> 0)&0xff,
				(uip >> 8)&0xff,
				(uip >> 16)&0xff,
				(uip >> 24)&0xff);

}

short Ip4Addr::port() const
{
	return ntohs(m_addr.sin_port);
}

unsigned int Ip4Addr::ipInt() const
{
	return ntohl(m_addr.sin_addr.s_addr);
}
bool Ip4Addr::isValid() const
{
	return m_addr.sin_addr.s_addr != INADDR_ANY;

}

}
