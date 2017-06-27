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
#include "buffer.h"

namespace txh{

using namespace std;

class TcpConn;
using TcpConnPtr = shared_ptr<TcpConn>;
using TcpCallback = function<void (const TcpConnPtr&)>;

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

    void connect(EventBase* base, const std::string& host, short port, int timeout, const std::string& localip);
    void attach(EventBase* base, int fd, Ip4Addr local, Ip4Addr peer);


    void onRead(const TcpCallback& cb) { assert(!readcb_); readcb_ = cb; };
    Buffer& getInput() { return input_; }
    void send(Buffer& msg);
private:
    EventBase *base_;
    Channel *channel_;
    Status state_;
    short destPort_;
    Ip4Addr local_,peer_;
    TcpCallback readcb_, writecb_, statecb_;
    Buffer input_,output_;


    void handleRead(const TcpConnPtr& con);
    void handleWrite(const TcpConnPtr& con);
    int handleHandshake(const TcpConnPtr& con);
    void cleanup(const TcpConnPtr& con);
    virtual int readImp(int fd, void* buf, size_t bytes);
    virtual int writeImp(int fd, const void* buf, size_t bytes);
    ssize_t isend(const char* buf, size_t len);
};


}




#endif /* TCPCONN_H_ */
