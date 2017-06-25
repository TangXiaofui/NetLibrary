#ifndef LOGGING_H
#define LOGGING_H

#include <sys/time.h>
#include <sys/syscall.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include <atomic>
#include <sys/stat.h>
namespace txh{
using namespace std;

#define hlog(level,...) 														\
do{ 																			\
	if(level < Logger::getLogger().getLogLevel()){ 								\
		Logger::getLogger().logv(level,__FILE__,__LINE__,__func__,__VA_ARGS__); \
	} 																			\
}while(0)

#define trace(...) 	hlog(Logger::LTRACE,__VA_ARGS__)
#define debug(...) 	hlog(Logger::LDEBUG,__VA_ARGS__)
#define info(...) 	hlog(Logger::LINFO,__VA_ARGS__)
#define warn(...) 	hlog(Logger::LWARN,__VA_ARGS__)
#define error(...) 	hlog(Logger::LERROR,__VA_ARGS__)
#define fatal(...) 	hlog(Logger::LFATAL,__VA_ARGS__)

#define fatalif(b,...) 	do{ if(b){ fatal(__VA_ARGS__); }}while(0)
#define exitif(b,...) 	do{ if(b){ error(__VA_ARGS__);exit(1);}}while(0)


struct Logger:private noncopyable{
	enum LogLevel{LFATAL = 0,LERROR,LUERR,LWARN,LINFO,LDEBUG,LTRACE,LALL};
	Logger();
	~Logger();
	void logv(int level,const char *fname,int line,const char *func, const char *fmt ...);
	
	void setFileName(const string &fname);
	void setLogLevel(const string &level);
	void setLogLevel(LogLevel level);

	LogLevel getLogLevel();
	const char *getLogLevelStr();
	int getFd();

	void adjustLogLevel(int adjust);
	void setRotateInterval(long rotateInterval);
	static Logger& getLogger();

private:
	void maybeRotate();
	static const char* levelStr[LALL+1];
	int m_fd;
	LogLevel m_level;
	long m_lastRotate;
	atomic<int64_t> m_realRotate;
	long m_rotateInterval;
	string m_filename;
};

}

#endif
