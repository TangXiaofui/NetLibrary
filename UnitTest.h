#ifndef UNITTEST_H
#define UNITTEST_H

#include <vector>
#include <sstream>
#include <iostream>
#include <memory>
#include <string.h>
namespace txh{
namespace test{
using namespace std;
	
namespace {
	struct Test{
		Test(const char* n, void (*f)()):name(n),func(f){};
		const char *name;
		void (*func)();
	};
shared_ptr<vector<Test>> tests = NULL;
}

bool RegisterTest(const char* name,void (*func)())
{
	if(tests == NULL)
		tests = make_shared<vector<Test> >();
	Test t(name,func);
	tests->push_back(t);	
}

void RunAllTests(const char* match)
{
	int nums = 0;
	if(tests->size())
	{
		for(size_t i = 0 ; i < tests->size() ; ++i)
		{
			Test &t = (*tests)[i];
			if(match != NULL)
			{
				if(strcmp(t.name,match) != 0)
				{
					continue;
				}
			}
			cout << "========== test " << t.name << endl;
			t.func();
			++nums;	
		}
		cout << "========== Passed " << nums << endl;
	}
}

#define CATNAME(a,b) a##b

#define TEST(name) 													\
class CATNAME(_Test_,name){ 										\
public: 															\
	static void _RunIt(){ 											\
		CATNAME(_Test_,name) t; 									\
		t.Run(); 													\
	} 																\
	void Run();														\
}; 																	\
bool CATNAME(_Test_ignore_,name) = 									\
::txh::test::RegisterTest(#name,&CATNAME(_Test_,name)::_RunIt);      \
void CATNAME(_Test_,name)::Run()
	

class Tester{
private:
	bool ok;
	const char* fname;
	int line;
	stringstream ss;
public:
	Tester(const char* f,int l):ok(true),fname(f),line(l){
	
	}
	~Tester()
	{
		if(!ok)
		{
				cout << "ASSERT FAILED "<< fname <<" " << line << " "  << ss.str() << endl;
				exit(1);
		}
	}
	Tester& Is(bool flag, const char*msg)
	{
		if(!flag)
		{
			ss << " " << msg ;
			ok = false;
		}
		return *this;
	}

#define BINARY_OP(name,op) 										\
template <typename X,typename Y> 								\
Tester& name(const X &x,const Y &y) 							\
{ 																\
	if(! (x op y)) 												\
	{ 															\
		ss << "fail " << x << (" " #op " " )  << y ; 			\
		ok = false;  											\
	} 															\
	return *this; 												\
} 								

BINARY_OP(IsEq,==)
#undef BINARY_OP

template <class X>
Tester& operator << (const X & x)
{
	if(!ok)
	{
		ss << " " << x;		
	}
	return *this;
}

#define ASSERT_TRUE(c) ::txh::test::Tester(__FILE__,__LINE__).Is(c,#c)
#define ASSERT_FALSE(c) ::txh::test::Tester(__FILE__,__LINE__).Is(!(c),#c)
#define ASSERT_EQ(x,y) ::txh::test::Tester(__FILE__,__LINE__).IsEq((x),(y))

};
}
}
#endif
