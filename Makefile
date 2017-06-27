CXXFLAGS =	 -g  -Wl,--no-as-needed  -pthread -std=c++11  -Wall -fmessage-length=0  

OBJS =		main.o slice.o logging.o util.o net.o port.o thread.o eventbase.o poller.o channel.o \
tcpServer.o tcpConn.o buffer.o

LIBS =

TARGET =	NetLibrary

$(TARGET):	$(OBJS)
	g++ -o $(TARGET) $(OBJS) $(LIBS) -pthread

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
