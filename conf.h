#ifndef CONF_H
#define CONF_H

#include <map>
#include <list>
#include <string>
#include <algorithm>
#include <stdlib.h>

namespace txh{
	using namespace std;

static string makeKey(const string &section,const string &name)
{
	string key = section + "." + name;
	transform(key.begin(),key.end(),key.begin(),::tolower);
	return key;	
}

namespace{
struct LineScanner{
	LineScanner(char *in):p(in),err(0){}
	LineScanner& skipSpace(){
		while(!err && *p && isspace(*p)){
			++p;	
		}
		return *this;
	}

	string rstrip(char *b, char *e){
		while(!err && e > b &&isspace(e[-1])){
			--e;	
		}	
		return string(b,e);
	}
	
	int peerChar(){
		skipSpace();
		return *p;	
	}
	
	LineScanner& skip(int i){
		p += i;
		return *this;	
	}
	
	LineScanner& match(char c){
		skipSpace();
		err = *p++ != c;
		return *this;	
	}

	string consumeTill(char c){
		skipSpace();
		char *e = p;
		while(!err && *e && *e != c){
			++e;	
		}	
		
		if(*e != c)
		{
			err = 1;
			return string("");	
		}
		char *s = p;
		p = e;
		return rstrip(s,e);		
	}

	string consumeTillEnd(){
		skipSpace();
		char *e = p;
		while(!err && *e && *e != '#' && *e != ';')
		{
			if(isspace(*e)){
				break;
			}
			++e;
		}
		char *b = p;
		p = e;
		return rstrip(b,e);
	}

	char *p;
	int err;
};
	
}

struct Conf{
	int parse(const string &filename);
	string get(const string &section, const string& name, string default_value);
	long getInteger(const string &section, const string& name, long default_value);
	double getReal(const string &section, const string& name, double default_value);
	bool getBoolean(const string &section, const string& name, bool default_value);		
	
	list<string> getStrings(const string& section, const string& name);

	map<string,list<string>> m_values;
	string m_filename;	
};	

int Conf::parse(const string &filename)
{
	m_filename = filename;
	FILE *fp = fopen(m_filename.c_str(),"r");
	if(!fp)
		return -1;
	unique_ptr<FILE,decltype(fclose)*> release1(fp,fclose);
	static const int MAXSIZE = 1024;
	char *buf = new char[MAXSIZE];
	unique_ptr<char[]> release2(buf);
	int lineno = 0;
	int err = 0;
	string section,key;
	while(!err && fgets(buf,MAXSIZE,fp) != NULL){
		++lineno;
		LineScanner ln(buf);
		int c = ln.peerChar();
		if(c == ';' || c == '#' || c == '\0'){
			continue;	
		}
		else if(c == '['){
			section = ln.skip(1).consumeTill(']');
			err = ln.match(']').err;
			key = "";
		}
		else if(isspace(buf[0])){
			if(key.empty()){
				err = 1;
			}
			else{
				m_values[makeKey(section,key)].push_back(ln.consumeTill('\0'));
			}
		}
		else{
			LineScanner ls(ln);
			key = ln.consumeTill('=');
			if(ln.peerChar() == '='){
		
			}
			else{
				ln = ls;
				key = ln.consumeTill(':');
				err = ln.match(':').err;
			}
			string value = ln.skip(1).consumeTillEnd();
			m_values[makeKey(section,key)].push_back(value);
		}
	}
	return err ? lineno : 0;
}

string Conf::get(const string &section, const string &name, string default_value)
{
	string key = makeKey(section,name);
	auto p = m_values.find(key);
	return p == m_values.end() ? default_value : p->second.back();	
}

long Conf::getInteger(const string &section, const string &name, long default_value)
{
	string res = get(section,name,"");
	const char *b = res.c_str();
	char *e;
	long n = strtol(b,&e,0);
	return e > b ? n : default_value;
}

double Conf::getReal(const string &section, const string &name, double default_value)
{
	string res = get(section,name,"");
	const char *b = res.c_str();
	char *e;
	double n = strtod(b,&e);
	
	return e > b ? n : default_value;
}

bool Conf::getBoolean(const string &section,const string &name, bool default_value)
{
	string res = get(section,name,"");
	transform(res.begin(),res.end(),res.begin(),::tolower);
	if(res == "true" || res == "yes" || res == "on" || res == "1")
		return true;
	else if(res == "false" || res == "no" || res == "off" || res == "0")
		return false;
	else 
		return default_value;
}

list<string> Conf::getStrings(const string& section,const string& name)
{
	string key = makeKey(section,name);
	auto p = m_values.find(key);
	return p == m_values.end() ? list<string>() : p->second;
}

}

#endif
