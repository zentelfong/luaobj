#pragma once
#include "LuaCommon.h"
#include <stdio.h>

class LuaException
{
public:
	LuaException(const std::string & w):m_what(w){}
	LuaException(const char* w):m_what(w){}

	LuaException(const char* w,const char* file,int line)
	{
		char buf[256];
		_snprintf(buf,sizeof(buf),"LuaException:%s file:%s line:%d",w,file,line);
		m_what=buf;
	}

	virtual ~LuaException(){}

	virtual const char* what()
	{
		return m_what.c_str();
	}
private:
	std::string m_what;
};

//´íÎó´¦Àíº¯Êý
typedef int (*LuaErrorHandler) (const char* err);




#define LUA_THROW_EXCEPTION(w) throw LuaException(w,__FILE__,__LINE__)
