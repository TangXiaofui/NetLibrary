#ifndef CHANNEL_H
#define CHANNEL_H
#include <atomic>
#include "net.h"
#include "logging.h"
#include "util.h"
#include "thread.h"
#include "poller.h"
#include "eventbase.h"
namespace txh{
using namespace std;

struct EventBase;
struct Channel:private noncopyable{
	Channel(EventBase *base,int fd,int event);
	~Channel();
	EventBase* getBase(){ return m_base; }
	int fd() { return m_fd; }
	int64_t id() { return m_id; }
	short events() { return m_events; }

	void close();

	void onRead(const Task& readfunc){  m_readfunc = readfunc; }
	void onWrite(const Task& writefunc){ m_writefunc = writefunc; }
	void onRead(const Task&& readfunc){ m_readfunc = move(readfunc); }
	void onWrite(const Task&& writefunc){ m_writefunc = move(writefunc); }

	void enableRead(bool enable);
	void enableWrite(bool enable);
	void enableReadWrite(bool readEnable,bool writeEnable);
	bool readEnable();
	bool writeEnable();

	void handleRead() { m_readfunc(); }
	void handleWrite() { m_writefunc(); }

protected:
	EventBase *m_base;
	int m_fd;
	PollerBase *m_poller;
	int64_t m_id;
	short m_events;
	Task m_readfunc;
	Task m_writefunc;

};

}
#endif
