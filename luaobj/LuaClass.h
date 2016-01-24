#pragma once
#include "LuaCommon.h"
#include "LuaObject.h"


template <typename T> class LuaClass {
  typedef struct { T *pT;bool owner; } userdataType;
  
public:
  typedef int (T::*mfp)(LuaFuncState &L);
  typedef struct { const char *name; mfp mfunc; } RegType;


  static void Register(lua_State* L)
  {
	Register(L,NULL);
  }


  static void Register(lua_State *L,const char* parentClassName) {
    lua_newtable(L);
    int methods = lua_gettop(L);

    // store method table in globals so that
    // scripts can add functions written in Lua.
	const char* find=strstr(T::className(),".");
	if (find)
	{
		lua_pushlstring(L,T::className(),find-T::className());
		lua_gettable(L,LUA_GLOBALSINDEX);
		if (!lua_istable(L,-1))
		{
			lua_newtable(L);
			lua_pushlstring(L,T::className(),find-T::className());
			lua_pushvalue(L, -2);
			lua_settable(L, LUA_GLOBALSINDEX);
		}
		assert(lua_type(L,-1)==LUA_TTABLE);
		lua_pushstring(L, find+1);
		lua_pushvalue(L, methods);
		lua_settable(L, -3);
		lua_pop(L,1);
	}
	else
	{
		lua_pushstring(L, T::className());
		lua_pushvalue(L, methods);
		lua_settable(L, LUA_GLOBALSINDEX);
	}
	


	if(parentClassName)
	{
		const char* find=strstr(parentClassName,".");
		if (find)
		{
			lua_pushlstring(L,parentClassName,find-parentClassName);
			lua_gettable(L,LUA_GLOBALSINDEX);
			lua_getfield(L,-1,find+1);
			lua_remove(L,-2);

			assert(lua_type(L,-1)==LUA_TTABLE);
			lua_setmetatable(L,methods);
		}
		else
		{
			lua_getglobal(L,parentClassName);
			assert(lua_type(L,-1)==LUA_TTABLE);
			lua_setmetatable(L,methods);
		}
	}


    lua_pushliteral(L, "__index");
	lua_pushvalue(L, methods);
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


    // fill method table with methods from class T
    for (RegType *l = T::methods(); l->name; l++) {
    /* edited by Snaily: shouldn't it be const RegType *l ... ? */
      lua_pushstring(L, l->name);
      lua_pushlightuserdata(L, (void*)l);
      lua_pushcclosure(L, thunk, 1);
      lua_settable(L, methods);
    }

    lua_pop(L,1);  // drop metatable and method table
  }


  static T* luaToC(LuaObject obj)
  {
	T * ptr=NULL;
	userdataType *ud=(userdataType*)obj.toData();
	if (ud)
		ptr= ud->pT;
	return ptr;
  }

  static LuaObject cToLua(LuaState* L,T* obj)
  {
    userdataType *ud = static_cast<userdataType*>(lua_newuserdata(L, sizeof(userdataType)));
    ud->pT = obj;  // store pointer to object in userdata
	ud->owner=false;

	const char* find=strstr(T::className(),".");
	if(find)
	{
		lua_pushlstring(L,T::className(),find-T::className());
		lua_gettable(L,LUA_GLOBALSINDEX);
		lua_getfield(L,-1,find+1);
		lua_remove(L,-2);
	}
	else
		lua_getglobal(L, T::className());  // lookup metatable in Lua registry
    lua_setmetatable(L, -2);
	return LuaObject(L->getLuaState(),lua_gettop(L->getLuaState()));
  }

private:
  LuaClass();  // hide default constructor

  // get userdata from Lua stack and return pointer to T object
  static T *check(lua_State *L, int narg) {
	T * ptr=NULL;
	userdataType *ud=(userdataType*)lua_touserdata(L,narg);
	if (ud)
		ptr= ud->pT;
	return ptr;
  }


  // member function dispatcher
  static int thunk(lua_State *L) 
  {
    // stack has userdata, followed by method args
    T *obj = check(L, 1);  // get 'self', or if you prefer, 'this'
    lua_remove(L, 1);  // remove self so member function args start at index 1
	if(!obj)
		return 0;
    RegType *l = static_cast<RegType*>(lua_touserdata(L, lua_upvalueindex(1)));
	LuaFuncState LS(L);
	return (obj->*(l->mfunc))(LS);
  }

  // create a new T object and
  // push onto the Lua stack a userdata containing a pointer to T object
  static int new_T(lua_State *l) {

	LuaFuncState L(l);
    T *obj = new T(L);  // call constructor for T objects
    userdataType *ud =static_cast<userdataType*>(lua_newuserdata(l, sizeof(userdataType)));
    ud->pT = obj;  // store pointer to object in userdata
	ud->owner=true;
	const char* find=strstr(T::className(),".");
	if(find)
	{
		lua_pushlstring(l,T::className(),find-T::className());
		lua_gettable(l,LUA_GLOBALSINDEX);
		lua_getfield(l,-1,find+1);
		lua_remove(l,-2);
	}
	else
		lua_getglobal(l, T::className());  // lookup metatable in Lua registry
    lua_setmetatable(l, -2);
    return 1;  // userdata containing pointer to T object
  }

  // garbage collection metamethod
  static int gc_T(lua_State *L) {
    userdataType *ud = static_cast<userdataType*>(lua_touserdata(L, 1));
    T *obj = ud->pT;
	if(ud->owner)
		delete obj;  // call destructor for T objects
    return 0;
  }

  static int tostring_T (lua_State *L) {
    char buff[32];
    userdataType *ud = static_cast<userdataType*>(lua_touserdata(L, 1));
    T *obj = ud->pT;
    sprintf(buff, "%p", obj);
    lua_pushfstring(L, "%s (%s)", T::className(), buff);
    return 1;
  }
};


#define DECLARE_METHOD(Name) {#Name, &Name}


//����ӳ��
#define BEGIN_MAP_FUNC(_class,_className)\
	static const char* className(){return _className;}\
	static LuaClass<_class>::RegType * methods()\
	{\
		static LuaClass<_class>::RegType _methods[] = {

#define END_MAP_FUNC\
		{0,0}\
		};\
		return _methods;\
	}
