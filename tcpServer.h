/*
 * tcpServer.h
 *
 *  Created on: Jun 23, 2017
 *      Author: txh
 */

#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include <memory>
#include <string>
#include "eventbase.h"
#include "util.h"
#include "net.h"
#include "channel.h"
#include "logging.h"

namespace txh{

using namespace std;
struct TcpServer;

using TcpServerPtr = shared_ptr<TcpServer>;

struct TcpServer:public noncopyable{

  TcpServer(EventBase *base);
  ~TcpServer();
  int bind(const string &addr,short port,bool reusePort);


  static TcpServerPtr startServer(EventBase *base, const string& addr,short port,bool reusePort);
  Ip4Addr getAddr();


private:
  EventBase *base_;
  Ip4Addr serverAddr_;
  Channel *listenChannel_;

  void handleAccept();
};

}


#endif /* TCPSERVER_H_ */
