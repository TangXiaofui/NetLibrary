#ifndef FILE_H
#define FILE_H
#include <string>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <vector>
#include <unistd.h>
#include "status.h"


namespace txh{

using namespace std;
struct File{
	static Status getContent(const string &fname,string &cont);
	static Status writeContent(const string &fname,const string &cont);
	static Status renameSave(const string &newName,const string &oldName,const string &cont);
	static Status getChildren(const string &dname,vector<string> *result);
	static Status deleteFile(const string &fname);
	static Status createDir(const string &dname);
	static Status deleteDir(const string &dname);
	static Status getFileSize(const string &fname,uint64_t *size);
	static Status renameFile(const string &src,const string &target);
	static bool fileExist(const string &fname);
};	


Status File::getContent(const string &fname,string &cont)
{
	int fd = open(fname.c_str(),O_RDONLY);
	if(fd < 0)
		return Status::ioError("open",fname);
	ExitCaller ecl([=]{close(fd);});
	int n;
	char buf[4096] = {0};
	for(;;)
	{
		n = read(fd,buf,4096);
		if(n < 0)
			return Status::ioError("read",fname);
		else if(n == 0)
			break;
		cont.append(buf,n);
	}
	return Status();
}

Status File::writeContent(const string &fname,const string &cont)
{
	int fd = open(fname.c_str(),O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR);
	if( fd < 0 )
		return Status::ioError("open",fname);
	
	ExitCaller ecl([=]{close(fd);});
	int r = write(fd,cont.data(),cont.size());
	if (r < 0)
		return Status::ioError("write",fname);
	return Status();	
}

Status File::renameSave(const string &newName,const string &oldName,const string &cont)
{
	Status s = writeContent(oldName,cont);
	if(s.ok())
	{
		unlink(newName.c_str());
		s = renameFile(oldName,newName);	
	}
	return s;
}

Status File::getChildren(const string &dname,vector<string> *result)
{
	if(result == NULL)
		return Status::ioError("result","is NULL");
	
	result->clear();

	DIR *dir = opendir(dname.c_str());
	if( dir == NULL)
		return Status::ioError("opendir",dname);
	
	struct dirent* entry;
	while( (entry = readdir(dir)) != NULL )
	{
		result->push_back(entry->d_name);
	}

	closedir(dir);
	return Status();
	
		
}

Status File::deleteFile(const string &fname)
{
	if(unlink(fname.c_str()) != 0)
		return Status::ioError("unlink",fname);
	return Status();	
}

Status File::createDir(const string &dname)
{
	if(mkdir(dname.c_str(),0755) != 0)
		return Status::ioError("mkdir",dname);
	return Status();	
}

Status File::deleteDir(const string &dname)
{
	if(rmdir(dname.c_str()) != 0)
		return Status::ioError("rmdir",dname);
	return Status();
}

Status File::getFileSize(const string &fname,uint64_t *size)
{
	if(size == 0)
		return Status::ioError("*size","=NULL");
	struct stat buf;
	if(stat(fname.c_str(),&buf) != 0)
	{
		return Status::ioError("stat",fname);
	}
	else
	{
		*size = buf.st_size;	
	}
	return Status();
}

Status File::renameFile(const string &fname,const string &target)
{
	if(rename(fname.c_str(),target.c_str()) != 0)
		return Status::ioError("rename",fname + " " + target);
	return Status();		
}

bool File::fileExist(const string &fname)
{
	return access(fname.c_str(),F_OK) == 0;
}

}
#endif
