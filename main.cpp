//============================================================================
// Name        : NetLibrary.cpp
// Author      : txh
// Version     :
// Copyright   : 
// Description : Hello World in C, Ansi-style
//============================================================================

#include "port.h"
#include "thread.h"
#include "UnitTest.h"
#include <arpa/inet.h>
#include "file.h"
#include "conf.h"
#include "util.h"
#include "status.h"
#include "daemon.h"
#include "logging.h"
#include "eventbase.h"
#include <thread>
#include "tcpServer.h"
//sudo tcpdump  -i lo -s0 -n -t 'src host 127.0.0.1' and 'tcp port 2099' -X -nn
//nc [IP|host] [port]
//nc -l -p [port]
//valgrind --tool=memcheck --leak-check=full ./
using namespace txh;
using namespace std;

TEST(TestTcpServer)
{
  EventBase base;
  Signal::signal(SIGINT,[&]{ base.exit(); });

  TcpServerPtr svr = TcpServer::startServer(&base,"",2099);
  exitif(svr == NULL, "start tcp server failed");

  svr->onConnRead([](const TcpConnPtr& con) {
    con->send(con->getInput());
  });

  base.loop();
}

TEST(TestEventbase)
{
	EventBase base;

	base.safeCall([]{
		debug("base add task");
	});
	thread t([&]{
		this_thread::sleep_for(chrono::seconds(3));
		debug("base exit");
		base.exit();
	});
	base.loop();
	t.join();
}

TEST(TestTimer)
{
	Logger::getLogger().setLogLevel("trace");
	EventBase base;
	long now = util::timeMilli();
	info("adding timer");
	TimerId tid1 = base.runAt(now+100,[&]{info("run at 100");});
	TimerId tid2 = base.runAfter(50,[]{info("timer after 50");});
	TimerId tid3 = base.runAfter(20,[]{info("timer interval 10");},10);
	base.runAfter(120,[&]{
		info("after 120 then cancel all");
		base.cancel(tid1);
		base.cancel(tid2);
	 	base.cancel(tid3);
		base.exit();
	});
	base.loop();
}

TEST(TestThread)
{
	ThreadPool pool(5,10,true);
	int processed = 0;
	int *p = &processed;
	int added = 0;

	for(int i = 0 ; i < 10 ; ++i)
	{
		added += pool.addTask([=]{ printf("task %d processed\n",++*p); });
	}
//	pool.start();
	printf("add another task,but not start\n");
	for(int i = 0 ; i < 10 ; ++i)
	{
		added += pool.addTask([=]{ printf("task %d processed\n",++*p); });
	}
	pool.exit();
	pool.join();
	printf("processed = %d , added = %d\n",processed,added);

}

TEST(TestNet)
{
	ASSERT_EQ(Ip4Addr("127.0.0.1",1234).toString(),"127.0.0.1:1234");
}

TEST(TestPort)
{
	uint16_t port = 0x1122;
	ASSERT_EQ(port::htobe(port),htons(port));
	uint32_t port2 = 0x11223344;
	ASSERT_EQ(port::htobe(port2),htonl(port2));
	struct in_addr ip4= port::getHostByName("127.1.1.1");
	char buf[INET_ADDRSTRLEN] = {0};
	inet_ntop(AF_INET,&ip4,buf,INET_ADDRSTRLEN);
	cout << buf << endl;
}

TEST(TestLog)
{
	Logger::getLogger().setFileName("test.log");
	Logger::getLogger().setLogLevel("trace");
	Logger::getLogger().setRotateInterval(5);
	int i = 0;
	while(1)
	{
		//debug("This is %s\n","debug");
		Logger::getLogger().logv(3,__FILE__,__LINE__,__func__,"this is a %d\n",i);
		sleep(1);
		i++;
		exitif(i == 10,"timeout");
	}

}

TEST(TestFile)
{
	ASSERT_EQ(File::fileExist("test"),true);
	ASSERT_EQ(File::fileExist("t"),false);
	uint64_t size;
	File::getFileSize("test",&size);
	cout << size << endl;
	File::createDir("abc");
	File::deleteDir("def");
	string cont;
	File::getContent("tags",cont);
	File::renameSave("tags_bak","tmp",cont);
	vector<string> res;
	File::getChildren("./",&res);
	for(auto r : res)
	{
		cout << r << endl;
	}
}

TEST(TestStatus)
{
	Status t(Status::ioError("+","t"));
	Status t2(move(Status::fromSystem()));
	Status t3;
	t3 = t2;
	Status t4;
	t4 = move(t3);
	cout << t4.code() << " " << t4.msg() << endl;
	cout << t4.toString().c_str() << endl;


}

TEST(TestConf)
{
	LineScanner ls((char*)" [section] ");
	string section = ls.skipSpace().skip(1).consumeTill(']');
	Conf f;
	f.parse("./test.ini");
	cout << f.getInteger("section3","key1",0) << endl;
	cout << f.getBoolean("section3","key4",false) << endl;
	cout << f.getReal("section3","key3",0.0) << endl;
}

TEST(TestSlice)
{
	char *str = (char *)" abc def hij k ";
	Slice ss(str);
	cout << ss.begin() << endl;
	ss.trimSpace();
	ss.eatWord();
	cout << ss.begin() << endl;
	ss.eat(1);
	cout << ss.begin() << endl;
	vector<Slice> res = ss.split(' ');
	for(auto r : res)
	{
		cout << string(r).data() << endl;
		cout << r.toString().data() << endl;
	}
}

TEST(TestUtil)
{
	time_t t;
	time(&t);
	cout << util::format("%s\n","test");
	cout <<	util::readableTime(t).data() << endl;
	cout << atoi("123abc") << endl;
	cout << util::timeMilli() << endl;
	ExitCaller test([](){ cout << "destory" << endl;});
	cout << "Test Return" << endl;
}

void quitfunc()
{
	exit(0);
}

int main(int argc,char *argv[])
{
	Logger::getLogger().setLogLevel("trace");
	if(argc > 1)
	{
		for(int i = 1 ; i < argc ;++i)
		{
			test::RunAllTests(argv[i]);
		}
/*		char buf[64] = {0};
		snprintf(buf,64,"%s.pid",argv[0]);
		Daemon::daemonProcess(argv[1],buf);
		Signal::signal(SIGQUIT,quitfunc);
		sleep(300);

*/	}
	else
	{
	    test::RunAllTests(NULL);
	}
	return 0;
}
