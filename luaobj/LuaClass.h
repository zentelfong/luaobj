#pragma once
#include "LuaCommon.h"
#include "LuaObject.h"
#include "LuaState.h"

template <typename T> class LuaClass
{
public:
	typedef T this_t;
	static T* toC(LuaObject obj)
	{
		if (obj.isUData())
		{
			LuaClass *ud = (LuaClass*)obj.toData();
			if (ud)
				return ud->pThis;
		}
		else if (obj.isTable())
		{
			LuaObject lud = LuaTable(obj)["__ptr"];
			LuaClass *ud = (LuaClass*)lud.toData();
			if (ud)
				return ud->pThis;
		}
		return NULL;
	}

	static LuaObject toLua(LuaState* Lp, T* obj, bool release = false)
	{
		lua_State* L = Lp->getLuaState();
		LuaClass *ud = static_cast<LuaClass*>(lua_newuserdata(L, sizeof(LuaClass)));
		ud->pThis = obj;  // store pointer to object in userdata
		ud->owner = release;

		const char* find = strstr(T::className(), ".");
		if (find)
		{
			lua_pushlstring(L, T::className(), find - T::className());
			lua_gettable(L, LUA_GLOBALSINDEX);
			lua_getfield(L, -1, find + 1);
			lua_remove(L, -2);
		}
		else
			lua_getglobal(L, T::className());  // lookup metatable in Lua registry
		lua_setmetatable(L, -2);
		return LuaObject(Lp, lua_gettop(L));
	}


public:
	T *pThis;
	bool owner;
};


template <typename T> class LuaRegister
{
public:
	typedef int (T::*mfp)(LuaFuncState &L);
	typedef struct { const char *name; mfp mfunc; } RegType;


	static void Register(lua_State* L)
	{
		Register(L, NULL);
	}

	template<class T2>
	static void Register(lua_State* L)
	{
		Register(L, T2::className());
	}


	static void Register(lua_State *L, const char* parentClassName) {
		lua_newtable(L);
		int methods = lua_gettop(L);

		// store method table in globals so that
		// scripts can add functions written in Lua.
		const char* find = strstr(T::className(), ".");
		if (find)
		{
			lua_pushlstring(L, T::className(), find - T::className());
			lua_gettable(L, LUA_GLOBALSINDEX);
			if (!lua_istable(L, -1))
			{
				lua_newtable(L);
				lua_pushlstring(L, T::className(), find - T::className());
				lua_pushvalue(L, -2);
				lua_settable(L, LUA_GLOBALSINDEX);
			}
			assert(lua_type(L, -1) == LUA_TTABLE);
			lua_pushstring(L, find + 1);
			lua_pushvalue(L, methods);
			lua_settable(L, -3);
			lua_pop(L, 1);
		}
		else
		{
			lua_pushstring(L, T::className());
			lua_pushvalue(L, methods);
			lua_settable(L, LUA_GLOBALSINDEX);
		}

		if (parentClassName)
		{
			const char* find = strstr(parentClassName, ".");
			if (find)
			{
				lua_pushlstring(L, parentClassName, find - parentClassName);
				lua_gettable(L, LUA_GLOBALSINDEX);
				lua_getfield(L, -1, find + 1);
				lua_remove(L, -2);

				assert(lua_type(L, -1) == LUA_TTABLE);
				lua_setmetatable(L, methods);
			}
			else
			{
				lua_getglobal(L, parentClassName);
				assert(lua_type(L, -1) == LUA_TTABLE);
				lua_setmetatable(L, methods);
			}
		}


		//lua_pushliteral(L, "__index");
		//lua_pushvalue(L, methods);

		lua_pushliteral(L, "__index");
		lua_pushvalue(L, methods);
		if (!lua_getmetatable(L, methods))
			lua_pushnil(L);
		lua_pushcclosure(L, index_T, 2);

		lua_settable(L, methods);

		lua_pushliteral(L, "__tostring");
		lua_pushcfunction(L, tostring_T);
		lua_settable(L, methods);

		lua_pushliteral(L, "__gc");
		lua_pushcfunction(L, gc_T);
		lua_settable(L, methods);

		lua_pushliteral(L, "new");
		lua_pushcfunction(L, new_T);
		lua_settable(L, methods);

		lua_pushliteral(L, "__ctype");
		lua_pushinteger(L, 1);
		lua_settable(L, methods);


		// fill method table with methods from class T
		for (RegType *l = T::methods(); l->name; l++) {
			/* edited by Snaily: shouldn't it be const RegType *l ... ? */
			lua_pushstring(L, l->name);
			lua_pushlightuserdata(L, (void*)l);
			lua_pushcclosure(L, thunk_T, 1);
			lua_settable(L, methods);
		}

		lua_pop(L, 1);  // drop metatable and method table
	}

private:
	LuaRegister();  // hide default constructor

	/*
	cls.__index=function(t,k)
	local ret=cls[k]
	if ret then
	return ret
	end
	ret=super[k]
	cls[k]=ret
	return ret
	end
	*/

	//lua_upvalueindex(1);//metatable
	//lua_upvalueindex(2);//supper
	static int index_T(lua_State *L)
	{
		//stack 1 self,2 key
		lua_pushvalue(L, 2);

		//stack 1 self,2 key,3 key
		lua_gettable(L, lua_upvalueindex(1));
		if (!lua_isnil(L, -1))
			return 1;

		//stack 1 self,2 key,3 nil
		lua_pop(L, 1);

		//supper为nil，直接返回空
		if (lua_isnil(L, lua_upvalueindex(2)))
			return 0;

		//stack 1 self,2 key
		lua_pushvalue(L, 2);

		//stack 1 self,2 key,3 key
		lua_gettable(L, lua_upvalueindex(2));
		if (lua_isnil(L, -1))
			return 0;

		//stack 1 self,2 key,3 data

		lua_pushvalue(L, 2);//key
		//stack 1 self,2 key,3 data ,4 key

		lua_pushvalue(L, -2);//value
		//stack 1 self,2 key,3 data ,4 key,5 data

		lua_settable(L, lua_upvalueindex(1));

		//stack 1 self,2 key,3 data 
		return 1;
	}

	// member function dispatcher
	static int thunk_T(lua_State *L)
	{
		// stack has userdata, followed by method args
		// get 'self', or if you prefer, 'this'
		T * obj = NULL;
		bool owner = false;

		if (lua_isuserdata(L, 1))
		{
			obj = (T*)lua_touserdata(L, 1);
		}
		else if (lua_istable(L, 1))
		{
			lua_getfield(L, 1, "__ptr");
			if (lua_isuserdata(L, -1))
			{
				obj = (T*)lua_touserdata(L, -1);
				lua_pop(L, 1);
			}
			else
			{
				assert(false);
			}
		}
		else
		{
			assert(false);
		}

		if (obj)
		{
			lua_remove(L, 1);  // remove self so member function args start at index 1
			RegType *l = static_cast<RegType*>(lua_touserdata(L, lua_upvalueindex(1)));
			LuaFuncState Ls(L, true);
			try{
				int rslt = (obj->*(l->mfunc))(Ls);
				return rslt;
			}
			catch (LuaException e)
			{
				lua_pushstring(L, e.what());
				lua_error(L);
				return 0;
			}
		}
		else
			return 0;
	}

	// create a new T object and
	// push onto the Lua stack a userdata containing a pointer to T object
	static int new_T(lua_State *l) {

		LuaFuncState L(l);
		//T *obj = new T(L);  // call constructor for T objects
		T *ud = static_cast<T*>(lua_newuserdata(l, sizeof(T)));
		ud->pThis = NULL;  // store pointer to object in userdata
		ud->owner = true;
		ud->ctor(L);//ud不是new出来的所以要显式调用构造函数
		const char* find = strstr(T::className(), ".");
		if (find)
		{
			lua_pushlstring(l, T::className(), find - T::className());
			lua_gettable(l, LUA_GLOBALSINDEX);
			lua_getfield(l, -1, find + 1);
			lua_remove(l, -2);
		}
		else
			lua_getglobal(l, T::className());  // lookup metatable in Lua registry
		lua_setmetatable(l, -2);
		return 1;  // userdata containing pointer to T object
	}

	// garbage collection metamethod
	static int gc_T(lua_State *L) {
		T *ud = static_cast<T*>(lua_touserdata(L, 1));
		if (ud)
		{
			if (ud->owner && ud->pThis)
			{
				delete ud->pThis;
				//ud->pThis->release();
				ud->pThis = NULL;
			}
		}
		return 0;
	}

	static int tostring_T(lua_State *L) {

		switch (lua_type(L, 1))
		{
		case LUA_TUSERDATA:
		{
			T *ud = static_cast<T*>(lua_touserdata(L, 1));
			lua_pushfstring(L, "%s (%p)", T::className(), ud->pThis);
		}
			break;
		case LUA_TTABLE:
		{
			lua_pushfstring(L, "table %p", lua_topointer(L, 1));
		}
			break;
		case LUA_TNIL:
		{
			lua_pushstring(L, "nil");
		}
			break;
		case LUA_TBOOLEAN:
		{
			if (lua_toboolean(L, 1))
				lua_pushstring(L, "true");
			else
				lua_pushstring(L, "false");
		}
			break;
		case LUA_TLIGHTUSERDATA:
		{
			lua_pushfstring(L, "light udata %p", lua_topointer(L, 1));
		}
			break;
		case LUA_TNUMBER:
		{
			lua_pushfstring(L, "number %f", lua_tonumber(L, 1));
		}
			break;
		case LUA_TSTRING:
		{
			lua_pushfstring(L, "string %s", lua_tostring(L, 1));
		}
			break;
		case LUA_TFUNCTION:
		{
			 lua_pushfstring(L, "function %p", lua_topointer(L, 1));
		}
			break;

		case LUA_TTHREAD:
		{
			lua_pushfstring(L, "thread %p", lua_topointer(L, 1));
		}
			break;
		}
		return 1;
	}
};


/*
class Test
{
public:
void test()
{
printf("test");
}
};

class LTest:public LuaClass<Test>
{
public:
LTest(LuaFuncState& L)
{
pThis=new Test();
}
int test(LuaFuncState& L)
{
pThis->test();
}
};

LuaClassRegister<LTest>::Register(L);
*/



#define DECLARE_FUNCEX(Name,Func) {Name, &selfClass::Func}

#define DECLARE_FUNC(Name) {#Name, &selfClass::Name}

#define BEGIN_MAP_FUNC(_class,_className) \
	static const char* className(){ return _className; }\
	typedef _class selfClass; \
	static LuaRegister<_class>::RegType * methods()\
{\
	static LuaRegister<_class>::RegType _methods[] = {


#define END_MAP_FUNC \
{ 0, 0 } \
}; \
	return _methods; \
}
