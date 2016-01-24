#include "LuaTable.h"
#include "LuaObjectImpl.h"


LuaTable::LuaTable(LuaState* L)
	:LuaObject(L)
{
	
}

LuaTable::LuaTable(LuaObjectImpl* impl)
	:LuaObject(impl)
{
	
}

LuaTable::LuaTable(const LuaObject& rfs)
	:LuaObject(rfs)
{
}

LuaTable::LuaTable(const LuaTable& rfs)
	:LuaObject(rfs)
{
}


bool LuaTable::isValid()
{
	return getType()==LUA_TTABLE;
}

LuaObject LuaTable::getTable(const char* key)
{
	if(isValid())
		return LuaObjectImpl::createGetTable(m_ptr->getCppLuaState(),m_ptr,key);
	else
		return luaNil;
}

LuaObject LuaTable::getTable(lua_Integer key)
{
	if(isValid())
		return LuaObjectImpl::createGetTable(m_ptr->getCppLuaState(),m_ptr,key);
	else
		return luaNil;
}

LuaObject LuaTable::operator[](const char* key)
{
	if(isValid())
		return LuaObjectImpl::createGetTable(m_ptr->getCppLuaState(),m_ptr,key);
	else
		return luaNil;
}

LuaObject LuaTable::operator[](lua_Integer idx)
{
	if(isValid())
		return LuaObjectImpl::createGetTable(m_ptr->getCppLuaState(),m_ptr,idx);
	else
		return luaNil;
}

bool LuaTable::setTable(const char* key,LuaObject val)
{
	if(isValid())
	{
		lua_State* L=m_ptr->getLuaState();
		lua_pushstring(L,key);//key
		if(val.isNone())
			lua_pushnil(L);
		else
			lua_pushvalue(L,val.getIndex());//value
		lua_settable(L,getIndex());
		return true;
	}
	return false;
}

bool LuaTable::setTable(lua_Integer key,LuaObject val)
{
	if(isValid())
	{
		lua_State* L=m_ptr->getLuaState();
		lua_pushinteger(L,key);//key
		if(val.isNone())
			lua_pushnil(L);
		else
			lua_pushvalue(L,val.getIndex());//value
		lua_settable(L,getIndex());
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
LuaTable::Iterator::Iterator()
	:m_valid(false)
{
}


LuaTable::Iterator::Iterator(LuaObject ltab)
	:m_table(ltab),m_valid(false)
{
	assert (m_table.isTable());
	if(m_table.isTable())
	{
		lua_State* L=ltab.getLuaState();
		lua_pushnil(L);
		if(lua_next(L,m_table.getIndex()))
		{
			m_valid=true;
			m_key=LuaObject(ltab.getCppLuaState(),lua_gettop(L)-1);
			m_value=LuaObject(ltab.getCppLuaState(),lua_gettop(L));
		}
	}
}

LuaObject LuaTable::Iterator::key()
{
	return m_key;
}

LuaObject LuaTable::Iterator::value()
{
	return m_value;
}

bool LuaTable::Iterator::operator++()
{
	if(m_table.isTable())
	{
		lua_State* L=m_table.getLuaState();
		lua_pushvalue(L,m_key.getIndex());
		if(lua_next(L,m_table.getIndex())!=0)
		{
			m_key=LuaObject(m_table.getCppLuaState(),lua_gettop(L)-1);
			m_value=LuaObject(m_table.getCppLuaState(),lua_gettop(L));
			m_valid=true;
			return true;
		}
	}
	m_valid=false;
	return false;
}

bool LuaTable::Iterator::operator==(const Iterator& rfs)
{
	if (m_valid==rfs.isValid())
	{
		if (m_valid)
		{
			return m_key==rfs.m_key;
		}
		else
			return true;
	}
	else
		return false;
}


bool LuaTable::Iterator::operator!=(const Iterator& rfs)
{
	return !operator==(rfs);
}

bool LuaTable::Iterator::isValid()const
{
	return m_valid;
}


LuaTable::Iterator LuaTable::begin()
{
	return Iterator(*this);
}

LuaTable::Iterator LuaTable::end()
{
	return Iterator();
}



















