/*
 * tcpConn.h
 *
 *  Created on: Jun 23, 2017
 *      Author: txh
 */

#ifndef TCPCONN_H_
#define TCPCONN_H_

#include <memory>
#include "eventbase.h"
#include "channel.h"
#include "util.h"
#include "net.h"
#include "logging.h"

namespace txh{

using namespace std;

class TcpConn;
using TcpConnPtr = shared_ptr<TcpConn>;

class TcpConn: public enable_shared_from_this<TcpConn>,public noncopyable{
    enum class Status{Invalid = 1,Handshaking,Connected,Closed,Failed,};
public:
    TcpConn();
    virtual ~TcpConn();

    template <typename C=TcpConn>
    static TcpConnPtr createConnect(EventBase *base,const string& host,short port,
                                    int timeout = 0,const string& localip = "");

    template <typename C=TcpConn>
    static TcpConnPtr createConnect(EventBase* base, int fd, Ip4Addr local, Ip4Addr peer);

private:
    EventBase *base_;
    Channel *channel_;
    Status state_;
    short destPort_;
    Ip4Addr local_,peer_;

    void connect(EventBase* base, const std::string& host, short port, int timeout, const std::string& localip);
    void attach(EventBase* base, int fd, Ip4Addr local, Ip4Addr peer);
    void handleRead(const TcpConnPtr& con);
    void handleWrite(const TcpConnPtr& con);
};


}




#endif /* TCPCONN_H_ */
