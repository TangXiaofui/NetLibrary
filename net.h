#ifndef NET_H
#define NET_H

#include "logging.h"
#include "slice.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <errno.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "port.h"
namespace txh{

using namespace std;

struct net{
	template<typename T> static T hton(T v){ return port::htobe(v); }	
	template<typename T> static T ntoh(T v){ return port::htobe(v); }
	static int setNonBlock(int fd,bool value = true);
	static int setReuseAddr(int fd, bool value = true);
	static int setReusePort(int fd, bool value = true);
	static int setNoDelay(int fd, bool value = true);
};	

struct Ip4Addr{
	Ip4Addr(const string &host,short port);
	Ip4Addr(short port = 0):Ip4Addr("",port){};
	Ip4Addr(const struct sockaddr_in &addr):m_addr(addr){};
	string toString() const;
	string ip() const;
	short port() const;
	unsigned int ipInt() const;
	bool isValid() const;
	struct sockaddr_in& getAddr() { return m_addr; }
	static string hostToIp(const string &host){
		Ip4Addr addr(host,0);
		return addr.ip();
	}

private:
	struct sockaddr_in m_addr;
};

}


#endif
