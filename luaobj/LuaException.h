#pragma once
#include "LuaCommon.h"
#include <stdio.h>

class LUAOBJ_API LuaException
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

#ifdef USE_CPP_EXCEPTION
#define LUA_THROW_EXCEPTION(w) throw LuaException(w,__FILE__,__LINE__)
#else

inline void print_lua_error_info(const char* w, const char* file, int line)
{
	printf("%s\n", w);
}

#define LUA_THROW_EXCEPTION(w) print_lua_error_info(w,__FILE__,__LINE__)
#endif
