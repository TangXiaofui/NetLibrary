/*
 * tcpConn.cpp
 *
 *  Created on: Jun 23, 2017
 *      Author: txh
 */


#include "tcpConn.h"

namespace txh{

TcpConn::TcpConn()
:base_(nullptr),channel_(nullptr),state_(Status::Invalid),destPort_(-1)
{

}
TcpConn::~TcpConn()
{
  trace("Destroy tcp from %s to %s",local_.toString().c_str(),peer_.toString().c_str());
  delete channel_;
}

template <typename C>
TcpConnPtr TcpConn::createConnect(EventBase *base,const string& host,short port,
				int timeout,const string& localip)
{
  TcpConnPtr conn(new C);
  conn->connect(base,host,port,timeout,localip);
  return conn;
}

template <typename C>
TcpConnPtr TcpConn::createConnect(EventBase* base, int fd, Ip4Addr local, Ip4Addr peer)
{
  TcpConnPtr conn(new C);
  conn->attach(base,fd,local,peer);
}


void TcpConn::connect(EventBase* base, const std::string& host, short port, int timeout, const std::string& localip)
{
  fatalif(state_ != Status::Invalid && state_ != Status::Closed && state_ != Status::Failed,
          "bad state while connect,state : %d",state_ );

}

void TcpConn::attach(EventBase* base, int fd, Ip4Addr local, Ip4Addr peer)
{
  fatalif((destPort_ <= 0 && state_ != Status::Invalid) || (destPort_ > 0 && state_ != Status::Handshaking),
          "you should use a new TcpConn to attach");
  base_ = base;
  state_ = Status::Handshaking;
  local_ = local;
  peer_ = peer;
  delete channel_;

  channel_ = new Channel(base_,fd,WriteEvent | ReadEvent);
  trace("tcp constructed from %s to %s, fd: %d",
        local.toString().c_str(),peer.toString().c_str());

  TcpConnPtr conn = shared_from_this();
  conn->channel_->onRead([=]{ conn->handleRead(conn);});
  conn->channel_->onWrite([=]{ conn->handleWrite(conn);});
}


int TcpConn::handleHandshake(const TcpConnPtr& con)
{
  fatalif(state_ != Status::Handshaking,"handleShaking call when state = %d",state_);
  struct pollfd pfd;
  pfd.fd = channel_->fd();
  pfd.events = POLLOUT | POLLERR;
  int r = poll(&pfd,1,0);
  if(r == 1 && pfd.revents == POLLOUT){
      channel_->enableReadWrite(true,false);
      state_ = Status::Connected;
      trace("tcp connected from %s to %s, fd = %d",
            local_.toString().c_str(),peer_.toString().c_str(),channel_->fd());
      //可以设置响应回调
  }
  else{
      trace("tcp handleshake poll fd: %d return revent:%d",channel_->fd(),pfd.revents);
      cleanup(con);
      return -1;
  }
  return 0;

}

void TcpConn::handleRead(const TcpConnPtr& con)
{
  if(state_ == Status::Handshaking && handleHandshake(con))
    return;

  while(state_ == Status::Connected){
      input_.makeRoom();
      int ret = 0;
      if(channel_->fd() >= 0){
	  ret = readImp(channel_->fd(),input_.end(),input_.space());
	  trace("channel %lld fd %d readed %d bytes", (long long)channel_->id(), channel_->fd(), ret);
      }
      if (ret == -1 && errno == EINTR) {
                  continue;
      }
      else if(ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)){
	  if (readcb_ && input_.size()) {
	      readcb_(con);
	  }
	  break;
      }
      else if (channel_->fd() == -1 || ret == 0 || ret == -1) {
           cleanup(con);
           break;
      }
      else {
           input_.addSize(ret);
     }
  }
}

void TcpConn::handleWrite(const TcpConnPtr& con)
{
  if(state_ == Status::Handshaking){
      handleHandshake(con);
  }
  else if(state_ == Status::Connected){
	///
  }
  else {
      error("handle write unexpected");
  }
}

void TcpConn::cleanup(const TcpConnPtr& con)
{
  if(readcb_ && input_.size())
    readcb_(con);

  if(state_ == Status::Handshaking)
    state_ = Status::Failed;
  else
    state_ = Status::Closed;

  trace("tcp closing from %s to %s , fd: %d ",
        local_.toString().c_str(),peer_.toString().c_str(),channel_->fd());

   readcb_ = writecb_ = statecb_ = nullptr;
//   Channel* ch = channel_;
//   channel_ = nullptr;
//   delete ch;
}

int TcpConn::readImp(int fd, void* buf, size_t bytes)
{
  return ::read(fd, buf, bytes);
}

int TcpConn::writeImp(int fd, const void* buf, size_t bytes)
{
  return ::write(fd, buf, bytes);
}

void TcpConn::send(Buffer& msg)
{
  if (channel_) {
       if (channel_->writeEnable()) { //just full
           output_.absorb(msg);
       }
       if (msg.size()) {
           ssize_t sended = isend(msg.begin(), msg.size());
           msg.consume(sended);
       }
//       if (msg.size()) {
//           output_.absorb(msg);
//           if (!channel_->writeEnable()) {
//               channel_->enableWrite(true);
//           }
//       }
   } else {
       warn("connection %s - %s closed, but still writing %lu bytes",
           local_.toString().c_str(), peer_.toString().c_str(), msg.size());
   }
}

ssize_t TcpConn::isend(const char* buf, size_t len)
{
  ssize_t sended = 0;
  while(sended < len){
      ssize_t ret = writeImp(channel_->fd(),buf+sended,len - sended);
      trace("channel %lld fd %d write %ld bytes", (long long)channel_->id(), channel_->fd(), ret);
      if(ret > 0){
	  sended += ret;
	  continue;
      }
      else if(ret == -1 && errno == EINTR){
	  continue;
      }
      else if(ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)){
	  if(! channel_->writeEnable())
	    channel_->enableWrite(true);
      }
      else{
	  error("write error: channel %lld fd %d wd %ld %d %s", (long long)channel_->id(), channel_->fd(), ret, errno, strerror(errno));
	              break;
      }
  }
  return sended;
}

}
