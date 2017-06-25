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
  channel_->onRead([=]{ conn->handleRead(conn);});
  channel_->onWrite([=]{ conn->handleWrite(conn);});
}

void TcpConn::handleRead(const TcpConnPtr& con)
{

}

void TcpConn::handleWrite(const TcpConnPtr& con)
{

}

}
