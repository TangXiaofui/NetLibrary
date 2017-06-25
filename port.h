#ifndef PORT_H
#define PORT_H

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <string>

namespace txh{
using namespace std;

namespace port{
	static const int kLittleEndian = LITTLE_ENDIAN;
	inline uint16_t htobe(uint16_t v){
		if(!kLittleEndian)
			return v;
		unsigned char *p = (unsigned char *)&v;
		return uint16_t(p[0]) << 8 | uint16_t(p[1]) ;
	}

	inline uint32_t htobe(uint32_t v){
		if(!kLittleEndian)
			return v;
		unsigned char *p = (unsigned char*)&v;
		return uint32_t(p[0]) << 24 | uint32_t(p[1]) << 16 | uint32_t(p[2]) << 8 | uint32_t(p[3]);
	}

	inline uint64_t htobe(uint64_t v){
		if(!kLittleEndian)
			return v;
		unsigned char *p = (unsigned char*)&v;
		return uint64_t(p[0]) << 56 | uint64_t(p[1]) << 48 | uint64_t(p[2]) << 40 | uint64_t(p[3]) << 32 | \
		 		uint64_t(p[4]) << 24 | uint64_t(p[5]) << 16 | uint64_t(p[6]) << 8 | uint64_t(p[7]);
	}

	inline int16_t htobe(int16_t v){  return (int16_t)htobe((uint16_t)v) ;}
	inline int32_t htobe(int32_t v){  return (int32_t)htobe((uint32_t)v) ;}
	inline int64_t htobe(int64_t v){  return (int64_t)htobe((uint64_t)v) ;}
	
	struct in_addr getHostByName(const string &hostname);
}
}
#endif
