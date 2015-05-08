
#ifndef COMMON_H
#define COMMON_H


extern "C" {
	#include "lua/lua.h"
	#include "lua/lualib.h"
	#include "lua/lauxlib.h"
}

#include <string.h>
#include <assert.h>
#include <string>
#include <vector>
#include "MemPool.h"
#include "Utf.h"

#define ZTLUA_NAME_OF_NAMESPACE luaobj

#define ZTLUA_NAMESPACE_BEGIN namespace ZTLUA_NAME_OF_NAMESPACE{
#define ZTLUA_NAMESPACE_END }

#define ZTLUA_NAMESPACE ZTLUA_NAME_OF_NAMESPACE

#define ZTLUA_USE_NAMESPACE using namespace ZTLUA_NAME_OF_NAMESPACE;

#ifdef WIN32
 #define TLSVAR			DWORD
 #define TLSALLOC(k)	(*(k)=TlsAlloc(), TLS_OUT_OF_INDEXES==*(k))
 #define TLSFREE(k)		(!TlsFree(k))
 #define TLSGET(k)		TlsGetValue(k)
 #define TLSSET(k, a)	(!TlsSetValue(k, a))
#ifdef _DEBUG
  static LPVOID ChkedTlsGetValue(DWORD idx)
  {
	LPVOID ret=TlsGetValue(idx);
	assert(S_OK==GetLastError());
	return ret;
  }
  #undef TLSGET
  #define TLSGET(k) ChkedTlsGetValue(k)
#endif
#else
 #define TLSVAR			pthread_key_t
 #define TLSALLOC(k)	pthread_key_create(k, 0)
 #define TLSFREE(k)		pthread_key_delete(k)
 #define TLSGET(k)		pthread_getspecific(k)
 #define TLSSET(k, a)	pthread_setspecific(k, a)
#endif



#define LUA_OK 0

class LuaState;
class LuaObject;
class LuaTable;


typedef int (*LuaCFunction) (LuaState& L,LuaTable& arg);

struct LuaReg
{
	const char* funcName;
	LuaCFunction func;
};


struct lua_index
{
	int index;
};


#endif
