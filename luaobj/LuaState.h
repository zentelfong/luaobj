#pragma once
#include "LuaCommon.h"
#include "LuaObject.h"
#include "LuaTable.h"
#include "LuaException.h"
#include "LuaStack.h"


class LUA_API LuaObjectStack
{
public:
	LuaObjectStack();
	~LuaObjectStack();

	void push(LuaObjectImpl* impl);
	void pop(LuaObjectImpl* impl);
private:
	void resize(int newSize);

	typedef LuaObjectImpl* data_t;

	data_t* m_data;
	int m_total;
	int m_use;
};

class LUA_API LuaMalloc
{
public:
	LuaMalloc(lua_State* L);
	void* malloc(size_t bytes);
	void  free(void* mem);
	void* realloc(void* mem, size_t newsize);
private:
	void* m_udata;
	lua_Alloc m_alloc;
};


class LUA_API LuaState
{
public:
	LuaState(lua_State* L);
	~LuaState(void);

	LuaMalloc getMalloc();

	LuaObject getGlobal(const char* name);
	void setGlobal(const char* name,LuaObject obj);

	/*
	与getGlobal不同的是支持以下
	lua中定义如下值：
	_G.test={a=1,b=2}
	
	C++中可以调用
	getField("test.a")
	*/
	LuaObject getField(const char* name);

	//TODO:impl this
	//void setField(const char* name,LuaObject obj);


	LuaObject getRegistery(const char* key);
	LuaObject getRegistery(int key);
	LuaObject getRegistery(void* key);
	void setRegistry(const char* key,const LuaObject obj);
	void setRegistry(int key,const LuaObject obj);
	void setRegistry(const void *key,const LuaObject obj);

	LuaObject newNil();
	LuaObject newNumber(lua_Number);
	LuaObject newInt(lua_Integer);

	LuaObject newString(const char*,int len=-1);

	LuaObject newString(const wchar_t*,int len=-1);

	LuaObject newPtr(void* p);//light userdata
	LuaObject newData(void* p,size_t sz);//userdata

	LuaObject newBool(bool);
	LuaObject newFunction(lua_CFunction);

	LuaObject newFunction(LuaCFunction);
	LuaTable newLib(const LuaReg lib[]);
	LuaObject require(const char *file);


	//do string
	LuaObject  doFile(const char *fileName);
	LuaObject  doString(const char *str);

	//返回一个函数
	LuaObject  loadFile(const char *fileName);
	LuaObject  loadString(const char *str);

	virtual void error(const char* errorMsg,...);

	//创建多个参数
	LuaTable newTable();


	lua_State* getLuaState(){return m_ls;}

	int getTop(){return lua_gettop(m_ls);}

	LuaObjectStack* getStack(){return &m_stack;}

	void enumStack();
protected:
	lua_State* m_ls;
	LuaObjectStack m_stack;
};


class LUA_API LuaFuncState:public LuaState
{
public:
	enum{
		LUA_MAX_ARG_COUNT=8
	};

	LuaFuncState(lua_State* L,bool bOwner=false);

	//获取函数参数0~argCount-1
	LuaObject arg(int count);
	int argNum();

	template<class T1>
	int lreturn(T1 t1)
	{
		StackOps::Push(m_ls,t1);
		return 1;
	}

	template<class T1,class T2>
	int lreturn(T1 t1,T2 t2)
	{
		StackOps::Push(m_ls,t1);
		StackOps::Push(m_ls,t2);
		return 2;
	}

	template<class T1,class T2,class T3>
	int lreturn(T1 t1,T2 t2,T3 t3)
	{
		StackOps::Push(m_ls,t1);
		StackOps::Push(m_ls,t2);
		StackOps::Push(m_ls,t3);
		return 3;
	}

	template<class T1,class T2,class T3,class T4>
	int lreturn(T1 t1,T2 t2,T3 t3,T4 t4)
	{
		StackOps::Push(m_ls,t1);
		StackOps::Push(m_ls,t2);
		StackOps::Push(m_ls,t3);
		StackOps::Push(m_ls,t4);
		return 4;
	}

	template<class T1,class T2,class T3,class T4,class T5>
	int lreturn(T1 t1,T2 t2,T3 t3,T4 t4,T5 t5)
	{
		StackOps::Push(m_ls,t1);
		StackOps::Push(m_ls,t2);
		StackOps::Push(m_ls,t3);
		StackOps::Push(m_ls,t4);
		StackOps::Push(m_ls,t5);
		return 5;
	}

	virtual void error(const char* errorMsg,...);
	bool isOwner(){return m_owner;}
private:
	LuaObject m_args[LUA_MAX_ARG_COUNT];
	int m_argCount;
	bool m_owner;
};


class LUA_API LuaOwnerState:public LuaState
{
public:
	LuaOwnerState();
	~LuaOwnerState();

	static void * luaAlloc(void *ud, void *ptr, size_t osize, size_t nsize);//内存分配

	MemPool* getMemPool(){return &m_pool;}

	virtual void error(const char* errorMsg,...);

	void setErrorHandler(LuaErrorHandler handler){m_errorHandler=handler;}
private:
	MemPool m_pool;//内存分配池
	LuaErrorHandler m_errorHandler;//错误处理函数
};



class LUA_API LuaAutoState:public LuaOwnerState
{
public:
	LuaAutoState();
	~LuaAutoState();

	static LuaAutoState* current();

private:
	static TlsValue s_tlsValue;
};

