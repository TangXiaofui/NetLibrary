#include "slice.h"

namespace txh{

Slice Slice::eatWord(){
	const char *b = pb;
	while(b < pe && isspace(*b) ){
		++b;
	}
	const char *e = b;
	while(e < pe && !isspace(*e) ){
		++e;	
	}
	pb = e;
	return Slice(b,e-b);
}

Slice Slice::eatLine(){
	const char *b = pb;
	while(pb < pe && *pb != '\r'&& *pb != '\n')
		++pb;
	return Slice(b,pb-b);	
}

Slice& Slice::trimSpace(){
	while(pb < pe && isspace(*pb))
		++pb;
	while(pb < pe && isspace(pe[-1]))
		--pe;
	return *this;
}
	
int Slice::compare(const Slice& rb) const {
	size_t sz = size(),rsz = rb.size();
	const size_t min_sz = sz > rsz ? rsz : sz;
	int r = memcmp(pb,rb.pb,min_sz);
	if(r == 0){
		if(sz > rsz)
			r = 1;
		else if(sz < rsz)
			r = -1;
	}
	return r;
}

bool Slice::startWith(const Slice &x) const {
	return (size() >= x.size()) && (memcmp(pb,x.pb,x.size()) == 0);	
}

bool Slice::endWith(const Slice &x) const {
	return (size() >= x.size()) && (memcmp(pe-x.size(),x.pb,x.size()) == 0);
}

vector<Slice> Slice::split(char ch) const {
	vector<Slice> res;
	const char *p1 = pb;
	for(const char *p2 = pb ; p2 != pe ; ++p2)
	{
		if(*p2 == ch)
		{
			res.push_back(Slice(p1,p2));
			p1 = p2+1;	
		}	
	}
	if(p1 != pe)
		res.push_back(Slice(p1,pe));
	return res;
}
}
