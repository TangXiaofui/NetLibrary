#ifndef UTIL_H
#define UTIL_H

#include <string.h>
#include <string>
#include <functional>
#include <memory>
#include <stdarg.h>
#include <unistd.h>
#include <ctime>
#include <chrono>
#include <fcntl.h>
namespace txh{
using namespace std;
struct noncopyable{
	noncopyable(){}
	noncopyable(const noncopyable&) = delete;
	noncopyable& operator = (const noncopyable &) = delete;
};	

struct util{
	static string format(const char* fmt,...);
	static int64_t timeMicro();
	static int64_t timeMilli();
	static int64_t steadyMicro();
	static int64_t steadyMilli();
	static string readableTime(time_t t);
	static int64_t atoi(const char *b, const char *e);
	static int64_t atoi2(const char *b,const char *e);
	static int64_t atoi(const char *b);
	static int addFdFlag(int fd,int flag);
};


struct ExitCaller:private noncopyable{
	ExitCaller(function<void()>&& f):functor(move(f)){}
	~ExitCaller(){
		functor();
	}	
private:
	function<void()> functor;	
};
}
#endif
