#include "port.h"

namespace txh{
using namespace std;
namespace port{    
	struct in_addr getHostByName(const std::string& host)
	{
        struct in_addr addr;
        char buf[1024];
        struct hostent hent;
        struct hostent *he = NULL;
        int herror = 0;
        memset(&hent,0,sizeof(hent));
        int r = gethostbyname_r(host.c_str(),&hent,buf,sizeof(buf),&he,&herror);
        if(r == 0 && he && he->h_addrtype == AF_INET)
        {
            addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        }
        else{
            addr.s_addr = INADDR_NONE;
        }
        return addr;
    }
}
}
