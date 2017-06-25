#ifndef STATUS_H
#define STATUS_H

#include <errno.h>
#include <stdarg.h>
#include <string>
#include "util.h"

namespace txh{
using namespace std;

inline const char* errstr(){ return strerror(errno);}

struct Status{
	Status():state(NULL){}
	Status(int code,const char *msg);
	Status(int code,const string msg):Status(code,msg.c_str()){}
	Status(const Status& r){ state = copyState(r.state); }
	~Status(){ delete[] state; }

	void operator = (const Status& r){ delete[] state; state = copyState(r.state); }
	Status(Status && r){ state = r.state; r.state = NULL; }
	void operator = (Status && r){delete[] state; state = r.state ; r.state = NULL; }
	
	static Status fromSystem(){ return Status(errno,strerror(errno)); }
	static Status fromSystem(int err){ return Status(err,strerror(err)); }
	static Status fromFormat(int code,const char* fmt,...);
	static Status ioError(const string &op,const string &name);

	int code(){ return state ? *(int32_t*)(state+4) : 0; }
	const char* msg() { return state ? state+8 : "" ;  }
	bool ok(){  return code() == 0;}
	string toString(){ return util::format("%d %s",code(),msg()); }
	
private:
	const char* state;
	const char* copyState(const char* state);
};	

Status::Status(int code,const char*msg)
{
	uint32_t sz = strlen(msg) + 8;
	char *res = new char[sz];

	*(uint32_t*)res = sz;
    *(int32_t *)(res+4) = code;
	memcpy(res+8,msg,sz-8);
	state = res;
}


const char* Status::copyState(const char* state)
{
	if(state == NULL)
		return state;
	uint32_t sz = *(uint32_t*)state;
	char *res = new char[sz];
	memcpy(res,state,sz);
	return res;
}

Status Status::fromFormat(int code, const char* fmt, ...)
{
	va_list ap;
	va_start(ap,fmt);
	uint32_t sz = vsnprintf(NULL,0,fmt,ap)+8 +1;
	va_end(ap);
	Status r;
	r.state = new char[sz];
	
	*(uint32_t*)r.state = sz;
	*(int32_t*)(r.state+4) = code;
	
	va_start(ap,fmt);
	vsnprintf((char*)r.state+8,sz-8,fmt,ap);
	va_end(ap);
	return r;
}

Status Status::ioError(const string &op,const string &name)
{
	return fromFormat(errno,"%s %s %s",op.c_str(),name.c_str(),errstr());	
}

}

#endif
