/*
 * tcpServer.cpp
 *
 *  Created on: Jun 23, 2017
 *      Author: txh
 */

#include "tcpServer.h"

namespace txh{

#define LISTEN 20


TcpServer::TcpServer(EventBase *base):
base_(base),
listenChannel_(nullptr)
{

}

 TcpServer::~TcpServer()
 {
   if(listenChannel_)
     delete listenChannel_;
 }

TcpServerPtr TcpServer::startServer(EventBase *base, const string& addr,short port,bool reusePort)
{
  TcpServerPtr p = make_shared<TcpServer>(base);
  int r = p->bind(addr,port,reusePort);
  if(r){
      error("bind to %s:%d failed %d %s", addr.c_str(), port, errno, strerror(errno));
  }
  return r == 0 ? p : nullptr;
}


int TcpServer::bind(const string &addr,short port,bool reusePort)
{
  serverAddr_ = Ip4Addr(addr,port);
  int fd = socket(AF_INET,SOCK_STREAM,0);
  int r = net::setReuseAddr(fd,true);
  fatalif(r, "set socket reuse option failed");
  r = net::setReusePort(fd,reusePort);
  fatalif(r,"set socket reuse port option failed");
  r = util::addFdFlag(fd,FD_CLOEXEC);
  fatalif(r,"addFdFlag FD_CLOEXEC failed");
  r = ::bind(fd,(struct sockaddr*)&serverAddr_.getAddr(),sizeof(struct sockaddr));
  if(r){
      ::close(fd);
      error("bind to %s failed %d %s", serverAddr_.toString().c_str(), errno, strerror(errno));
             return errno;
  }
  r = listen(fd,LISTEN);
  fatalif(r, "listen failed %d %s", errno, strerror(errno));
  info("fd %d listening at %s", fd, serverAddr_.toString().c_str());
  listenChannel_ = new Channel(base_,fd,ReadEvent);
  listenChannel_->onRead([this]{ this->handleAccept();});
  return 0;
}
void TcpServer::handleAccept()
{
  struct sockaddr_in cliaddr;
  socklen_t clilen = sizeof(cliaddr);
  int listenfd = listenChannel_->fd();
  int clifd;
  while(listenfd > 0 && (clifd = ::accept(listenfd,(struct sockaddr*)&cliaddr,&clilen)) >= 0){
      struct sockaddr_in peer,local;
      socklen_t len = sizeof(peer);
      int r = getpeername(clifd,(struct sockaddr *)&peer,&len);
      if(r < 0){
	  error("get peer name failed %d %s", errno, strerror(errno));
	              continue;
      }
      r = getsockname(clifd,(struct sockaddr *)&local,&len);
      if (r < 0) {
          error("getsockname failed %d %s", errno, strerror(errno));
                  continue;
      }
      r = util::addFdFlag(clifd, FD_CLOEXEC);
      fatalif(r, "addFdFlag FD_CLOEXEC failed");

      //client
      TcpConnPtr con = TcpConnPtr(new TcpConn);
      auto addcon = [=] {
	  con->attach(base_, clifd, local, peer);

	  if(readcb_)
	    con->onRead(readcb_);

      };

      base_->safeCall(move(addcon));


      if (listenfd >= 0) {
          info("accept return %d ", clifd);
      }
  }
}

Ip4Addr TcpServer::getAddr()
{
  return serverAddr_;
}

}


