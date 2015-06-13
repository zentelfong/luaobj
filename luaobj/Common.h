
#ifndef COMMON_H
#define COMMON_H

#ifdef LUAOBJ_USE_LUAJIT
extern "C" {
	#include "luajit/lua.h"
	#include "luajit/lualib.h"
	#include "luajit/lauxlib.h"
}

#ifdef WIN32
#param comment(lib,"lua51.lib")
#endif

#else
extern "C" {
	#include "lua/lua.h"
	#include "lua/lualib.h"
	#include "lua/lauxlib.h"
}
#endif

#include <string.h>
#include <assert.h>
#include <string>
#include <vector>


#define LUA_OK 0

class LuaState;
class LuaTable;

typedef LuaTable (*LuaCFunction) (LuaState *L,LuaTable* arg);



struct lua_index
{
	int index;
};

#endif
