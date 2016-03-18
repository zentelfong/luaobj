#include "LuaObject.h"
#include "LuaTable.h"
#include "LuaObjectImpl.h"

LuaObject luaNil((LuaObjectImpl*)NULL);

LuaObject::LuaObject()
{
	m_ptr=NULL;
}

LuaObject::LuaObject(LuaState* L)
{
	m_ptr=LuaObjectImpl::create(L);
}

LuaObject::LuaObject(LuaState* L,int idx)
{
	m_ptr=LuaObjectImpl::createFromIndex(L,idx);
}

LuaObject::LuaObject(LuaObjectImpl* impl)
{
	m_ptr=impl;
}

LuaObject::LuaObject(const LuaObject& p)
	: m_ptr(p.m_ptr) 
{
	if (m_ptr)m_ptr->addRef();
}

LuaObject::~LuaObject(void)
{
	if (m_ptr) m_ptr->release();
}

int LuaObject::getIndex()const
{
	if(m_ptr)
		return m_ptr->getIndex();
	else
		return 0;
}

const char* LuaObject::typeName()const
{
	if (m_ptr)
		return lua_typename(m_ptr->getLuaState(),m_ptr->getIndex());
	else
		return "None";
}

int  LuaObject::getType()const
{
	if (m_ptr)
		return lua_type(m_ptr->getLuaState(),m_ptr->getIndex());
	else
		return LUA_TNONE;
}

bool LuaObject::isNumber()const
{
	return getType()==LUA_TNUMBER;
}
bool LuaObject::isString()const
{
	return getType()==LUA_TSTRING;
}
bool LuaObject::isCFunction()const
{
	return getType()==LUA_TFUNCTION;
}
bool LuaObject::isFunction()const
{
	return getType()==LUA_TFUNCTION;
}
bool LuaObject::isUData()const
{
	return getType()==LUA_TUSERDATA;
}

bool LuaObject::isPtr()const
{
	return getType()==LUA_TLIGHTUSERDATA;
}

bool LuaObject::isTable()const
{
	return getType()==LUA_TTABLE;
}
bool LuaObject::isNil()const
{
	return getType()==LUA_TNIL;
}
bool LuaObject::isBool()const
{
	return getType()==LUA_TBOOLEAN;
}
bool LuaObject::isThread()const
{
	return getType()==LUA_TTHREAD;
}
bool LuaObject::isNone()const
{
	return getType()==LUA_TNONE;
}

lua_Number LuaObject::toNumber()const
{
	if(m_ptr)
		return lua_tonumber(m_ptr->getLuaState(),m_ptr->getIndex());
	else
		return 0;
}

float LuaObject::toFloat()const
{
	if(m_ptr)
		return (float)lua_tonumber(m_ptr->getLuaState(),m_ptr->getIndex());
	else
		return 0;
}

lua_Integer LuaObject::toInt()const
{
	if(m_ptr)
		return lua_tointeger(m_ptr->getLuaState(),m_ptr->getIndex());
	else
		return 0;
}

unsigned int LuaObject::toUInt()const
{
	if(m_ptr)
	{
		lua_Integer rt=lua_tointeger(m_ptr->getLuaState(),m_ptr->getIndex());
		if(rt>=0)
			return (unsigned int)rt;
		else
			return 0;
	}
	else
		return 0;
}

bool LuaObject::toBool()const
{
	if(m_ptr)
		return lua_toboolean(m_ptr->getLuaState(),m_ptr->getIndex())!=0;
	else
		return false;
}
const char* LuaObject::toString()const
{
	if(m_ptr)
		return lua_tostring(m_ptr->getLuaState(),m_ptr->getIndex());
	else
		return "";
}
lua_CFunction LuaObject::toCFunction()const
{
	if(m_ptr)
		return lua_tocfunction(m_ptr->getLuaState(),m_ptr->getIndex());
	else
		return NULL;
}
void * LuaObject::toData()const
{
	if(m_ptr)
		return lua_touserdata(m_ptr->getLuaState(),m_ptr->getIndex());
	else
		return NULL;
}

lua_State* LuaObject::toThread()const
{
	if(m_ptr)
		return lua_tothread(m_ptr->getLuaState(),m_ptr->getIndex());
	else
		return NULL;
}
const void* LuaObject::toPtr()const
{
	if(m_ptr)
		return lua_topointer(m_ptr->getLuaState(),m_ptr->getIndex());
	else
		return NULL;
}

size_t LuaObject::objLen()const
{
	if(m_ptr)
		return lua_objlen(m_ptr->getLuaState(),m_ptr->getIndex());
	else
		return 0;
}


LuaObject& LuaObject::operator=(const LuaObject& other)
{
	if(m_ptr)
	{
		if(other.m_ptr)
		{
			lua_pushvalue(m_ptr->getLuaState(),other.getIndex());
			lua_replace(m_ptr->getLuaState(),m_ptr->getIndex());		
		}
		else
		{
			lua_pushnil(m_ptr->getLuaState());
			lua_replace(m_ptr->getLuaState(),m_ptr->getIndex());
		}
	}
	else if(other.m_ptr)
	{
		m_ptr=other.m_ptr;
		m_ptr->addRef();
	}
	return *this;
}

bool LuaObject::operator==(lua_Integer n)
{
	return toInt()==n;
}

bool LuaObject::operator==(const char* s)
{
	if(isString() && s)
		return strcmp(toString(),s)==0;
	else
		return false;
}

bool LuaObject::operator==(const LuaObject& other)
{
	if (m_ptr && other.m_ptr)
	{
		return lua_equal(getLuaState(),getIndex(),other.getIndex())==0;
	}
	else
	{
		return m_ptr==other.m_ptr;
	}
}


lua_State* LuaObject::getLuaState()
{
	if(m_ptr)
		return m_ptr->getLuaState();
	else
		return NULL;
}

LuaState* LuaObject::getCppLuaState()
{
	if (m_ptr)
		return m_ptr->getCppLuaState();
	else
		return NULL;
}


bool LuaObject::setMetatable(LuaTable tab)
{
	if(m_ptr && tab.isValid())
	{
		lua_State * L=getLuaState();
		lua_pushvalue(L,tab.getIndex());
		return lua_setmetatable(L,getIndex())==LUA_OK;
	}
	return false;
}

LuaTable LuaObject::getMetatable()
{
	if(m_ptr)
	{
		lua_State * L=getLuaState();
		lua_getmetatable(L,getIndex());
		return LuaObject(getCppLuaState(),lua_gettop(L));	
	}
	else
		return luaNil;
}





