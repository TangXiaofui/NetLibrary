#include "logging.h"

namespace txh{
	
Logger::Logger():
m_level(LINFO),
m_lastRotate(time(NULL)),
m_rotateInterval(86400)
{
	tzset();
	m_fd = -1;
	m_realRotate = m_lastRotate;	
}

Logger::~Logger()
{
	if(m_fd != -1)
	{
		close(m_fd);		
	}
}

const char* Logger::levelStr[LALL+1] = {
	"FATAL",
	"ERROR",
	"UERR",
	"WARN",
	"INFO",
	"DEBUG",
	"TRACE",
	"ALL",
};
static thread_local uint64_t tid;
void Logger::logv(int level,const char *name,int line, const char *func,const char *fmt ...)
{
	if(tid == 0)
	{
		tid = syscall(SYS_gettid);
	}
	if(level > m_level)
		return ;
	maybeRotate();

	char buf[4*1024];
	char *pb = buf;
	char *pe = buf + sizeof(buf);
	struct timeval now_tv;
	gettimeofday(&now_tv,NULL);
	const time_t seconds = now_tv.tv_sec;
	struct tm t;
	localtime_r(&seconds,&t);
	pb += snprintf(pb,pe-pb,
	"%04d/%02d/%02d-%02d:%02d:%02d:%06d %lx %s %s:%d  ",
	t.tm_year+1900,
	t.tm_mon+1,
	t.tm_mday,
	t.tm_hour,
	t.tm_min,
	t.tm_sec,
	static_cast<int>(now_tv.tv_usec),
	(long)tid,
	levelStr[level],
	name,
	line );
	va_list ap;
	va_start(ap,fmt);
	pb += vsnprintf(pb,pe-pb,fmt,ap);
	va_end(ap);
	pb = min(pb,pe-2);
	while(*--pb == '\n')
	{
	}
	*++pb = '\n';
	*++pb = '\0';
	int fd = m_fd == -1 ? 1: m_fd;
	int err = write(fd,buf,pb-buf);
	if(err != pb-buf)
	{
		fprintf(stderr,"write file fail %s %s %d",__FILE__,__func__,__LINE__);
		return ;	
	}
}

void Logger::setLogLevel(const string &level)
{
	LogLevel ilevel = LINFO;
	for(int i = 0 ; i < sizeof(levelStr)/sizeof(const char*) ; ++i)
	{
		if(strcasecmp(level.c_str(),levelStr[i]) == 0)
		{
			ilevel = (LogLevel)i;
			break;
		}
	}
	setLogLevel(ilevel);
}

void Logger::setLogLevel(LogLevel level)
{
	m_level = min(LALL,max(LFATAL,level));
}

Logger::LogLevel Logger::getLogLevel()
{
	return m_level;
}

const char* Logger::getLogLevelStr()
{
	return levelStr[m_level];
}

int Logger::getFd()
{
	return m_fd;
}

void Logger::adjustLogLevel(int adjust)
{
	setLogLevel(LogLevel(m_level+adjust));
}

void Logger::setRotateInterval(long rotateInterval)
{
	m_rotateInterval = rotateInterval;	
}

Logger& Logger::getLogger()
{
	static Logger logger;
	return logger;	
}

void Logger::setFileName(const string &fname)
{
	int fd = open(fname.c_str(),O_APPEND|O_CREAT|O_WRONLY|O_CLOEXEC,DEFFILEMODE);
	if(fd < 0)
	{
		fprintf(stderr,"open %s failed",fname.c_str());
		return ;
	}
	m_filename = fname;
	if(m_fd == -1)
		m_fd = fd;
	else
	{
		int r = dup2(fd,m_fd);
		if(r < 0)
			fprintf(stderr,"dup2 fail"); 
		close(fd);
	}
}

void Logger::maybeRotate()
{
	time_t now = time(NULL);
	if(m_filename.empty() || (now - timezone)/m_rotateInterval == (m_lastRotate - timezone)/m_rotateInterval)
		return;
	m_lastRotate = now;
	long old = m_realRotate.exchange(now);
	if((old - timezone)/m_rotateInterval == (m_lastRotate - timezone)/m_rotateInterval)
		return ;
	struct tm ntm;
	localtime_r(&now,&ntm);
	char newName[1024*4];
	snprintf(newName,1024*4,"%s.%d%02d%02d%02d%02d",
	m_filename.c_str(),ntm.tm_year+1900,ntm.tm_mon+1,ntm.tm_mday,ntm.tm_hour,ntm.tm_min);

	const char* oldName = m_filename.c_str();
	int err = rename(oldName,newName);
	if(err != 0)
	{
		fprintf(stderr,"rename fail %s %s %d",__FILE__,__func__,__LINE__);
		return ;
	}
	int fd = open(m_filename.c_str(),O_APPEND|O_CREAT | O_WRONLY | O_CLOEXEC,DEFFILEMODE);
	if(fd < 0)
	{
		fprintf(stderr,"open file fail %s %s %d",__FILE__,__func__,__LINE__);
		return ;
	}

	dup2(fd,m_fd);
	close(fd);

}
}
