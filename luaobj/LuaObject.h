﻿#pragma once
#include "LuaCommon.h"
#include "LuaStack.h"

class LuaObjectImpl;
class LuaTable;

class LUAOBJ_API LuaObject
{
public:
	LuaObject();
	LuaObject(LuaState* L);
	LuaObject(LuaState* L,int idx);
	LuaObject(LuaObjectImpl* impl);
	LuaObject(const LuaObject& rfs);
	~LuaObject(void);

	int getIndex()const;


	//类型判断
	const char* typeName()const;
	int  getType()const;
	bool isNumber()const;
	bool isString()const;
	bool isCFunction()const;
	bool isFunction()const;
	bool isUData()const;
	bool isTable()const;
	bool isNil()const;
	bool isBool()const;
	bool isThread()const;
	bool isNone()const;

	//lightudata
	bool isPtr()const;

	//to
	lua_Number   toNumber()const;
	float		 toFloat()const;
	lua_Integer  toInt()const;
	unsigned int toUInt()const;
	bool		 toBool()const;
	const char*  toString()const;
	lua_CFunction toCFunction()const;
	void *		 toData()const;

	lua_State*   toThread()const;
	const void*  toPtr()const;

	size_t		 objLen()const;//返回数组的元素个数，字符串的长度，userdata所占字节数等

	//操作符
	LuaObject& operator=(const LuaObject& other);

	//比较
	bool operator==(lua_Integer n);
	bool operator==(const char* s);
	bool operator==(const LuaObject& other);


	lua_State* getLuaState();
	LuaState* getCppLuaState();

	bool setMetatable(LuaTable tab);
	LuaTable getMetatable();

protected:
	template<class T> friend  class LuaClass;

	LuaObjectImpl * m_ptr;
	friend class LuaObjectImpl;
};


LUAOBJ_API extern LuaObject luaNil;//lua nil

namespace StackOps
{
	inline void Push(lua_State* L, LuaObject value)	
	{
		lua_pushvalue(L,value.getIndex()); 
	}
}
