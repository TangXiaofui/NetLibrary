#include "util.h"

namespace txh{

string util::format(const char *fmt, ...)
{
	char buffer[10];
	char *base = NULL;
	unique_ptr<char[]> release1;
	for(int i = 0 ; i < 2 ; ++i )
	{
		int bufsize;
		if(i == 0)
		{
			bufsize = sizeof(buffer);
			base = buffer;	
		}
		else
		{
			bufsize = 1000;
			base = new char[bufsize];
			release1.reset(base);
		}
		
		char *p = base;
		char *limit = base + bufsize;

		if(p < limit)
		{
			va_list ap;
			va_start(ap,fmt);
			p += vsnprintf(p,limit-p,fmt,ap);
			va_end(ap);
		}

		if(p >= limit)
		{
			if(i == 0)
				continue;
			else
			{
				p = limit-1;
				*p = '\0';	
			}	
		}
		break;
	}
	return base;
			
}


int64_t util::timeMicro()
{
	chrono::time_point<chrono::system_clock> p = chrono::system_clock::now();
	return chrono::duration_cast<chrono::microseconds>(p.time_since_epoch()).count();	
}
int64_t util::timeMilli()
{
	return timeMicro()/1000;
}
int64_t util::steadyMicro()
{
	chrono::time_point<chrono::steady_clock> p = chrono::steady_clock::now();
	return chrono::duration_cast<chrono::microseconds>(p.time_since_epoch()).count();
}
int64_t util::steadyMilli()
{
	return steadyMicro()/1000;	
}
string util::readableTime(time_t t)
{
	struct tm tm1;
	localtime_r(&t,&tm1);
	return format("%04d-%02d-%02d %02d:%02d:%02d",
	tm1.tm_year+1900,tm1.tm_mon+1,tm1.tm_mday,tm1.tm_hour,tm1.tm_min,tm1.tm_sec);

}
int64_t util::atoi(const char *b, const char *e)
{
	return strtol(b,(char**)&e,10);
}
int64_t util::atoi2(const char *b,const char *e)
{
	char **ne = (char**)&e;
	int64_t v = strtol(b,ne,10);
	return ne == (char**)&e ? v : -1;
}
int64_t util::atoi(const char *b)
{
	return atoi(b,b+strlen(b));
}
int util::addFdFlag(int fd,int flag)
{
	int ret = fcntl(fd,F_GETFD);
	return fcntl(fd,F_SETFD,ret|flag);	
}
}
