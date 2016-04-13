#pragma once
#include "LuaObject.h"

//ÖØÔØ²Ù×÷·û
class LuaInt:public LuaObject
{
public:
	LuaInt(const LuaObject& rfs)
		:LuaObject(rfs)
	{}
	LuaInt(const LuaInt& rfs)
		:LuaObject(rfs)
	{}
	bool isValid(){return isNumber();}
	operator int(){return toInt();}
};

class LuaBool:public LuaObject
{
public:
	LuaBool(const LuaObject& rfs)
		:LuaObject(rfs)
	{}
	LuaBool(const LuaInt& rfs)
		:LuaObject(rfs)
	{}
	bool isValid(){return true;}
	operator bool(){return toBool();}
};

class LuaString:public LuaObject
{
public:
	LuaString(const LuaObject& rfs)
		:LuaObject(rfs)
	{}
	LuaString(const LuaInt& rfs)
		:LuaObject(rfs)
	{}
	bool isValid(){return isString();}
	operator const char*(){return toString();}
};

class LuaDouble:public LuaObject
{
public:
	LuaDouble(const LuaObject& rfs)
		:LuaObject(rfs)
	{}
	LuaDouble(const LuaInt& rfs)
		:LuaObject(rfs)
	{}
	bool isValid(){return isNumber();}
	operator double(){return toNumber();}
};

class LuaPtr:public LuaObject
{
public:
	LuaPtr(const LuaObject& rfs)
		:LuaObject(rfs)
	{}
	LuaPtr(const LuaInt& rfs)
		:LuaObject(rfs)
	{}
	bool isValid(){return isPtr();}
	operator const void*(){return toPtr();}
};







