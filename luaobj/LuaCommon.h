#pragma once

extern "C" {
	#include "../lua/lua.h"
	#include "../lua/lualib.h"
	#include "../lua/lauxlib.h"
}

#include "LuaMacro.h"
#include <string.h>
#include <string>
#include "Utf.h"


class LUAOBJ_API TlsValue
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
