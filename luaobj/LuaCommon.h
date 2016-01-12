#pragma once

extern "C" {
	#include "lua/lua.h"
	#include "lua/lualib.h"
	#include "lua/lauxlib.h"
}

#include <string.h>
#include <assert.h>
#include <string>
#include "MemPool.h"
#include "Utf.h"

#ifdef WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif


#define ZTLUA_NAME_OF_NAMESPACE luaobj

#ifdef WIN32
 #define TLSVAR			DWORD
 #define TLSALLOC(k)	(*(k)=TlsAlloc(), TLS_OUT_OF_INDEXES==*(k))
 #define TLSFREE(k)		(!TlsFree(k))
 #define TLSSET(k, a)	(!TlsSetValue(k, a))

 #ifdef DEBUG
  inline LPVOID CheckedTlsGetValue(DWORD idx)
  {
	LPVOID ret=TlsGetValue(idx);
	assert(S_OK==GetLastError());
	return ret;
  }
  #define TLSGET(k) CheckedTlsGetValue(k)
 #else
  #define TLSGET(k)		TlsGetValue(k)
 #endif

#else
 #define TLSVAR			pthread_key_t
 #define TLSALLOC(k)	pthread_key_create(k, 0)
 #define TLSFREE(k)		pthread_key_delete(k)
 #define TLSGET(k)		pthread_getspecific(k)
 #define TLSSET(k, a)	pthread_setspecific(k, a)
#endif

class TlsValue
{
public:
	TlsValue()
	{
		TLSALLOC(&m_key);
	}
	~TlsValue()
	{
		TLSFREE(m_key);
	}

	inline bool set(void* value)
	{
		return TLSSET(m_key, value);
	}

	inline void* get()
	{
		return TLSGET(m_key);
	}
private:
	TLSVAR m_key;
};


#define LUA_OK 0

class LuaState;
class LuaFuncState;
class LuaObject;
class LuaTable;


typedef int (*LuaCFunction) (LuaFuncState& L);

struct LuaReg
{
	const char* funcName;
	LuaCFunction func;
};


struct lua_index
{
	int index;
};
