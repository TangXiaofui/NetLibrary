#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <string>
#include <fcntl.h>
#include <map>
using namespace std;

#define exit_if(b,...) if(b){printf(__VA_ARGS__);fprintf(stderr,"%s : %d errno:%d ,errno msg:%s \n",__FILE__,__LINE__,errno,strerror(errno));exit(1);}
string httpstr;

void setNonBlock(int fd)
{
	int flags = fcntl(fd,F_GETFL,0);
	exit_if(flags < 0,"fcntl get flags failed");
	
	int r = fcntl(fd,F_SETFL,flags | O_NONBLOCK);
	exit_if(r < 0,"fcntl set flags fail");	
}

void updateEvents(int epollfd,int fd,int event,int op)
{
	struct epoll_event ev;
	memset(&ev,0,sizeof ev);
	
	ev.events = event;
	ev.data.fd = fd;
	printf("%s fd %d events read %d write %d\n",op == EPOLL_CTL_ADD?"add":"mod",fd,event & EPOLLIN,event & EPOLLOUT);
	
	int r = epoll_ctl(epollfd,op,fd,&ev);
	exit_if(r,"epoll ctl failed");	
}


void handleAccept(int efd,int sfd)
{
	int clisock;
	struct sockaddr_in cliaddr;
	socklen_t clilen = sizeof(cliaddr);
	clisock = accept(sfd,(struct sockaddr *)&cliaddr,&clilen);
	exit_if(clisock < 0, "accept error");

	printf("accept a connection from %s \n",inet_ntoa(cliaddr.sin_addr));

	setNonBlock(clisock);
	updateEvents(efd,clisock,EPOLLIN,EPOLL_CTL_ADD);		
}

struct Con{
	string readed;
	size_t written;
	bool writeEnable;
	Con():written(0),writeEnable(false){}
};
map<int,Con> cons;

void sendRes(int efd,int fd)
{
	Con &con = cons[fd];
	size_t left = httpstr.length() - con.written;
	int wd;
	while((wd = write(fd,httpstr.data()+con.written,left))>0){
		con.written += wd;
		left -= wd;
		printf("write %d bytes left:%d bytes\n",wd,left);	
	}
	if(left == 0){
		cons.erase(fd);
		return ;	
	}		
	if(wd < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)){
		return ;	
	}

	if (wd<=0) {
        printf("write error for %d: %d %s\n", fd, errno, strerror(errno));
        close(fd);
        cons.erase(fd);
    }

}

void handleRead(int efd,int fd)
{
	char buf[4096] = {0};
	int n = 0;
	while((n = read(fd,buf,sizeof buf)) > 0){
		string &readed = cons[fd].readed;
		readed.append(buf,n);
		if(readed.length() > 4){
			if(readed.substr(readed.length()-2,2) == "\n\n" || 
				readed.substr(readed.length()-4,4) == "\r\n\r\n"){
				sendRes(efd,fd);	
			}
		}
	}

	if(n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
		return ;
	if(n < 0){
		printf("read fd %d fail,errno %d,errno msg %s\n",fd,errno,strerror(errno));	
	}
	close(fd);
	cons.erase(fd);
}

void handleWrite(int efd,int fd)
{
	sendRes(efd,fd);	
}


void once_loop(int efd,int sfd,int waitMs)
{
	static const int kMaxEvents = 20;
	struct epoll_event actEvents[20];
	
	int n = epoll_wait(efd,actEvents,kMaxEvents,waitMs);
	printf("epoll_wait return n = %d\n",n);
	
	for(int i = 0 ; i < n ;++i){
		int fd = actEvents[i].data.fd;
		int event = actEvents[i].events;
		if(event & (EPOLLIN | EPOLLERR)){
			if(fd == sfd){
				handleAccept(efd,fd);
			}
			else
				handleRead(efd,fd);
		}	
		else if(event & EPOLLOUT)
			handleWrite(efd,fd);
		else
			exit_if(1,"unknow event");
	}	
}

int main(int argc,char *argv[])
{
	signal(SIGPIPE,SIG_IGN);
	httpstr = "HTTP/1.1 200 OK\r\nConnection: Keep-Alive\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: 1048576\r\n\r\n123456";
//	for(int i = 0 ; i < 1048570 ;i++)
//	{
//		httpstr+='\0';
//	}
	int sersock;
	short port = 8081;
	socklen_t serlen;
	struct sockaddr_in seraddr;
	
	int epollfd = epoll_create(1);
	exit_if(epollfd < 0,"create epoll fail");

	sersock = socket(AF_INET,SOCK_STREAM,0);
	exit_if(sersock<0,"socket error");

	bzero(&seraddr,sizeof(seraddr));
	seraddr.sin_family = AF_INET;
	seraddr.sin_addr.s_addr = INADDR_ANY;
	seraddr.sin_port = htons(port);
	
	serlen = sizeof(seraddr);
	int r = bind(sersock,(struct sockaddr*)&seraddr,serlen);
	exit_if(r, "bind to port %d",port);

	r = listen(sersock,20);
	exit_if(r,"listen failed");
	printf("fd %d listening at port %d\n",sersock,port);

	setNonBlock(sersock);
	updateEvents(epollfd,sersock,EPOLLIN,EPOLL_CTL_ADD);
	for(;;){
		once_loop(epollfd,sersock,10000);
	}
	return 0;
}
