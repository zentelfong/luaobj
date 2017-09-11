#pragma once
#include "LuaCommon.h"
#include "LuaObject.h"
#include "LuaState.h"

#ifndef LUAOBJ_CLASS_NAME
#define LUAOBJ_CLASS_NAME(Name) \
public: \
	virtual const char* className(){ return Name; } \
	static inline const char*  sClassName(){ return Name; } \

#endif

template <typename T> class LuaClass
{
public:
	typedef T this_t;

	//如果子类未实现则调用该实现
	void ctor(LuaFuncState&)
	{
		pThis = new this_t();
	}

	void instance(LuaFuncState&)
	{
	}

	void dtor()
	{
		delete pThis;
	}

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

	static LuaObject toLua(LuaState* L, T* obj)
	{
		if(!obj)
			return L->newNil();

		LuaTable lCache=getObjCacheTable(L);

		LuaObject lobj=lCache.getTable((void*)obj);
		if (lobj.isUData())
		{
			LuaClass* ud=static_cast<LuaClass*>(lobj.toData());
			if(ud->pThis==obj)
			{
				return lobj;
			}
			assert(ud->pThis==NULL);
		}

		lobj=L->newData(sizeof(LuaClass));
		LuaClass* ud=static_cast<LuaClass*>(lobj.toData());
		ud->pThis=obj;
		lobj.setMetatable(L->getField(obj->className()));
		lCache.setTable(obj,lobj);
		return lobj;
	}


	static void deleteByC(LuaState* L,T* obj)
	{
		LuaTable lCache = getObjCacheTable(L);
		LuaObject lobj=lCache.getTable((void*)obj);
		if (lobj.isUData())
		{
			LuaClass* ud=static_cast<LuaClass*>(lobj.toData());
			ud->pThis=NULL;//lua中调用前会检测该指针
		}
	}

	static LuaTable getObjCacheTable(LuaState* L)
	{
		LuaTable lCache=L->getRegistery("LuaObjCache");
		if(!lCache.isTable())
		{
			lCache=L->newTable();
			lCache.setTable("__mode","v");
			lCache.setMetatable(lCache);
			L->setRegistry("LuaObjCache",lCache);
		}
		return lCache;
	}

	static const char* className()
	{
		const char* name=T::sClassName();
		return T::sClassName();
	}
public:
	T *pThis;
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


	static void Register(lua_State *L, const char* parentClassName) 
	{
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


	static void RegisterStatic(lua_State* L)
	{
		RegisterStatic(L, NULL);
	}

	template<class T2>
	static void RegisterStatic(lua_State* L)
	{
		RegisterStatic(L, T2::className());
	}

	static void RegisterStatic(lua_State *L, const char* parentClassName) 
	{
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

		lua_pushliteral(L, "__index");
		lua_pushvalue(L, methods);
		if (!lua_getmetatable(L, methods))
			lua_pushnil(L);
		lua_pushcclosure(L, index_T, 2);

		lua_settable(L, methods);

		lua_pushliteral(L, "__tostring");
		lua_pushcfunction(L, tostring_T);
		lua_settable(L, methods);

		lua_pushliteral(L, "instance");
		lua_pushcfunction(L, instance_T);
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

		//pThis可能会被c++释放掉，而lua中任然包含该引用，故检测下
		if (obj && obj->pThis)
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
	static int new_T(lua_State *l) 
	{
		LuaFuncState L(l);
		LuaObject lud = L.newData(sizeof(T));
		T *ud =static_cast<T*>(lud.toData());
		ud->pThis = NULL;  // store pointer to object in userdata
		ud->ctor(L);

		LuaTable lMeta=L.getField(T::className());
		if(!lMeta.isTable())
		{
			lMeta=L.newTable();
			L.setField(T::className(),lMeta);
		}
		lud.setMetatable(lMeta);

		//设置到cache表中
		LuaTable lcache=T::getObjCacheTable(&L);
		lcache.setTable((void*)ud->pThis,lud);

		return L.lreturn(lud);
	}

	static int instance_T(lua_State *l) 
	{
		LuaFuncState L(l);
		LuaObject lud = L.newData(sizeof(T));
		T *ud =static_cast<T*>(lud.toData());
		ud->pThis = NULL;  // store pointer to object in userdata
		ud->instance(L);

		LuaTable lMeta=L.getField(T::className());
		if(!lMeta.isTable())
		{
			lMeta=L.newTable();
			L.setField(T::className(),lMeta);
		}
		lud.setMetatable(lMeta);

		//设置到cache表中
		LuaTable lcache=T::getObjCacheTable(&L);
		lcache.setTable((void*)ud->pThis,lud);

		return L.lreturn(lud);
	}

	// garbage collection metamethod
	static int gc_T(lua_State *L) 
	{
		T *ud = static_cast<T*>(lua_touserdata(L, 1));
		if (ud)
		{
			if (ud->pThis)
			{
				ud->dtor();
				ud->pThis = NULL;
			}
		}
		return 0;
	}

	static int tostring_T(lua_State *L) 
	{

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

#define BEGIN_MAP_FUNC(_class) \
	typedef _class selfClass; \
	static LuaRegister<_class>::RegType * methods()\
{\
	static LuaRegister<_class>::RegType _methods[] = {


#define END_MAP_FUNC \
{ 0, 0 } \
}; \
	return _methods; \
}
