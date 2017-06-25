#ifndef SLICE_H
#define SLICE_H

#include <vector>
#include <string.h>
#include <string>

namespace txh{
	using namespace std;

class Slice{
public:
	Slice():pb(""){ pe = pb;}
	Slice(const char *b,const char *e):pb(b),pe(e){}
	Slice(const char *b,size_t n):pb(b),pe(b+n){}
	Slice(const string &str):pb(str.data()),pe(str.data() + str.size()){}
	Slice(const char *b):pb(b),pe(b+strlen(b)){}

	const char* data() const { return pb ;}
	const char* begin() const { return pb ;}
	const char* end() const { return pe;}
	char front(){ return *pb; }
	char back() { return pe[-1]; }
	size_t size() const{ return pe - pb; }
	void resize(size_t sz){ pe = pb + sz; }
	inline bool empty() const { return pb == pe; }
	void clear(){ pb = pe = ""; }

	Slice eatWord();
	Slice eatLine();
	Slice eat(int sz) { Slice t(pb,sz); pb += sz; return t;}
	Slice sub(int boff,int sz){ Slice s(*this); s.pb += boff; s.pe = s.pb + sz ; return s;}
	Slice& trimSpace();

	inline char operator[](size_t n){ return pb[n]; }

	string toString() const { return string(pb,pe); }

	int compare(const Slice & rb )const;

	bool startWith(const Slice &x)const;
	bool endWith(const Slice &x)const;
	operator string() const {return string(pb,pe);}
	vector<Slice> split(char ch) const;
private:
	const char *pb;
	const char *pe;		
};

}

#endif
