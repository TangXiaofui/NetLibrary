#ifndef DAEMON_H
#define DAEMON_H

#include <functional>
#include <signal.h>
#include <map>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include "util.h"
#include <string.h>
#include <string>

namespace txh{
using namespace std;

struct Daemon{
	static int daemonStart(const char* pidfile);
	static int daemonRestart(const char *pidfile);
	static int daemonStop(const char *pidfile);
	static int getPidFromFile(const char *pidfile);

	static void daemonProcess(const char *cmd,const char *pidfile);
//	static void changeTo(const char *argv[]);

};

static int WritePidtoFile(const char* pidfile)
{
	char buf[32] = {0};
	int fp = open(pidfile,O_WRONLY|O_CREAT|O_TRUNC,0600);
	if(fp < 0 || lockf(fp,F_TLOCK,0) < 0 )
		fprintf(stderr,"Can't write Pid file\n");
	
	ExitCaller call1([=]{close(fp);});
	sprintf(buf,"%d\n",getpid());		
	ssize_t len = strlen(buf);
	ssize_t ret = write(fp,buf,len);
	if(ret != len)
	{
		fprintf(stderr,"Write pid file fail\n");
		return -1;	
	}
	return 0;
}

int Daemon::getPidFromFile(const char *pidfile)
{
	char buf[32] = {0};	
	char *p;
	int fd = open(pidfile,O_RDONLY);
	if(fd < 0)
		return fd;
	ssize_t res = read(fd,buf,sizeof(buf));
	close(fd);
	if(res <= 0)
		return -1;
	p = strchr(buf,'\n');
	if(p != NULL)
		*p = '\0';
	return atoi(buf);
}

int Daemon::daemonStart(const char *pidfile)
{
	int pid = getPidFromFile(pidfile);
	if(pid > 0 )
	{
		if(errno == EPERM || kill(pid,0) == 0)
			fprintf(stderr,"daemon exists,use restart\n");
			return -1;	
	}

	if(getppid() == 1)
	{
		fprintf(stderr,"already daemon,can't start\n");
		return -1;	
	}

	pid = fork();
	if(pid < 0)
	{
		fprintf(stderr,"fork error");
		return -1;
	}
	
	if(pid > 0)
		exit(0);	

	setsid();
	int r = WritePidtoFile(pidfile);
	if(r != 0)
		return r;
	
	int fd = open("/dev/null",0);
	if(fd > 0)
	{
		close(0);
		dup2(fd,0);
		dup2(fd,1);
		close(fd);
		string strpidfile = pidfile;
		static ExitCaller del([=]{ unlink(strpidfile.c_str());});
		return 0;	
	}
	return -1;
}


int Daemon::daemonStop(const char *pidfile)
{
	int pid = getPidFromFile(pidfile);
	if(pid <= 0)
	{
		fprintf(stderr," %s daemon not exist",pidfile);
		return -1;	
	}

	int r = kill(pid,SIGQUIT);
	if(r < 0)
	{
		fprintf(stderr,"program %d not exist",pid);
		return -1;	
	}
	for(int i = 0 ; i < 300; ++i)
	{
		usleep(10);
		r = kill(pid,SIGQUIT);
		if(r != 0)
		{
			fprintf(stderr,"program %d exit ",pid);
			return 0;	
		}
	}
	fprintf(stderr,"signal sended , but still exist after 3 seconds");
	return -1;
}

int Daemon::daemonRestart(const char *pidfile)
{
	int pid = getPidFromFile(pidfile);
	if(pid > 0)
	{
		if(kill(pid,0) == 0)
		{
			int r = daemonStop(pidfile);
			if(r < 0)
				return r;
				
		}
		else if(errno == EPERM)
		{
			fprintf(stderr,"don't have permission to kill %d programs",pid);
			return -1;	
		}
	}

	fprintf(stderr,"pidfile not valid");
	return daemonStart(pidfile);
}

void Daemon::daemonProcess(const char *cmd,const char* pidfile)
{
	int r = 0;
	if(cmd == NULL || strcmp(cmd,"start") == 0)
		r = daemonStart(pidfile);
	else if(strcmp(cmd,"stop") == 0)
	{
		r = daemonStop(pidfile);
		if(r == 0)
		{
			exit(0);	
		}
	}
	else if(strcmp(cmd,"restart") == 0)
		r= daemonRestart(pidfile);
	else
	{
		fprintf(stderr,"unknow cmd %s ",cmd);
		r = -1;
	}
	if(r)
		exit(1);
	
}

namespace{
	map<int,function<void()>> handlers;
	void signal_handler(int sig){
		handlers[sig]();
	}	
}

struct Signal{
	static void signal(int sig,const function<void()> &handler);
};	
void Signal::signal(int sig,const function<void()> &handler)
{
	handlers[sig] = handler;
	::signal(sig,signal_handler);
}

}
#endif
