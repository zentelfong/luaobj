#include "LuaState.h"
#include "LuaObjectImpl.h"
#include "LuaTable.h"
#include "LuaFunction.h"
#include "Utf.h"

#define MIN_STACK_SIZE 64

LuaObjectStack::LuaObjectStack()
	:m_data(NULL),m_total(0),m_use(0)
{
}

LuaObjectStack::~LuaObjectStack()
{
	free(m_data);
}

void LuaObjectStack::resize(int newSize)
{
	if (m_total<MIN_STACK_SIZE)
		m_total=MIN_STACK_SIZE;

	while (m_total<newSize)
	{
		m_total*=2;
	}
	m_data=(data_t*)realloc(m_data,m_total * sizeof(data_t));
}


void LuaObjectStack::push(LuaObjectImpl* impl)
{
	if (m_use+1 > m_total)
	{
		resize(m_use+1);
	}
	m_data[m_use++]=impl;
}

void LuaObjectStack::pop(LuaObjectImpl* impl)
{
	int find =-1;
	for (int i=m_use-1;i>=0;--i)
	{
		if (m_data[i]==impl)
		{
			find=i;
			break;
		}
		else
			m_data[i]->decIndex();
	}

	if (find>=0)
	{
		for (int j=find;j<m_use-1;++j)
		{
			m_data[j]=m_data[j+1];
		}
		--m_use;
	}
	else
	{
		assert(false);
	}
}





LuaMalloc::LuaMalloc(lua_State* L)
{
	m_alloc=lua_getallocf(L,(void**)&m_udata);
}

void* LuaMalloc::malloc(size_t bytes)
{
	return m_alloc(m_udata,NULL,0,bytes);
}

void  LuaMalloc::free(void* mem)
{
	m_alloc(m_udata,mem,0,0);
}

void* LuaMalloc::realloc(void* mem, size_t newsize)
{
	return m_alloc(m_udata,mem,0,newsize); 
}


LuaState::LuaState(lua_State* L)
	:m_ls(L),m_stack()
{
}


LuaState::~LuaState(void)
{
}

LuaMalloc LuaState::getMalloc()
{
	return LuaMalloc(m_ls);
}

LuaObject LuaState::getGlobal(const char* name)
{
	return LuaObjectImpl::createGetGlobal(this,name);
}

LuaObject LuaState::getRegistery(const char* key)
{
	return LuaObjectImpl::createGetRegistery(this,key);
}

LuaObject LuaState::getRegistery(int key)
{
	return LuaObjectImpl::createGetRegistery(this,key);
}


LuaObject LuaState::getRegistery(void* key)
{
	return LuaObjectImpl::createGetRegistery(this,key);
}

void LuaState::setRegistry(const char* key,const LuaObject obj)
{
	lua_pushstring(m_ls,key);
	lua_pushvalue(m_ls,obj.getIndex());
	lua_settable(m_ls, LUA_REGISTRYINDEX);
}

void LuaState::setRegistry(int key,const LuaObject obj)
{
	lua_pushinteger(m_ls,key);
	lua_pushvalue(m_ls,obj.getIndex());
	lua_settable(m_ls, LUA_REGISTRYINDEX);
}

void LuaState::setRegistry(const void *key,const LuaObject obj)
{
	lua_pushlightuserdata(m_ls,const_cast<void*>(key));
	lua_pushvalue(m_ls,obj.getIndex());
	lua_settable(m_ls, LUA_REGISTRYINDEX);
}



void LuaState::setGlobal(const char* name,LuaObject obj)
{
	lua_pushvalue(m_ls,obj.getIndex());
	lua_setglobal(m_ls,name);
}

const char* getFieldName(const char* str,char* buf,int len)
{
	int i=0;
	while (*str)
	{
		if (*str=='.')
		{
			break;
		}
		else if(i<len)
			buf[i++]=*str;
		++str;
	}
	buf[i]='\0';
	if (*str=='.')
		++str;
	return str;
}


LuaObject LuaState::getField(const char* name)
{
	char buf[128];
	name=getFieldName(name,buf,sizeof(buf));
	LuaTable lg=getGlobal(buf);
	
	while (lg.isValid() && *name)
	{
		name=getFieldName(name,buf,sizeof(buf));
		lg=lg.getTable(buf);
	}

	if(*name=='\0')
		return lg;
	else
		return luaNil;
}





LuaObject LuaState::newNil()
{
	return LuaObjectImpl::create(this);
}

LuaObject LuaState::newNumber(lua_Number num)
{
	return LuaObjectImpl::create(this,num);
}

LuaObject LuaState::newInt(lua_Integer num)
{
	return LuaObjectImpl::create(this,num);
}


LuaObject LuaState::newString(const char*str,int len)
{
	return LuaObjectImpl::create(this,str,len);
}

LuaObject LuaState::newString(const wchar_t* utf16,int len)
{
	int utf8len=UTF16To8(NULL,0,(unsigned short*)utf16,len);
	UtfBuffer<128> buf;
	buf.malloc((utf8len+1)*sizeof(char));
	UTF16To8((char*)buf.getBuf(),utf8len+1,(unsigned short*)utf16,utf8len+1);
	return LuaObjectImpl::create(this,(char*)buf.getBuf(),utf8len);
}



LuaObject LuaState::newPtr(void* p)
{
	return LuaObjectImpl::create(this,p);
}

LuaObject LuaState::newData(void* p,size_t sz)
{
	return LuaObjectImpl::create(this,p,sz);
}


LuaObject LuaState::newBool(bool b)
{
	return LuaObjectImpl::create(this,b);
}

LuaObject LuaState::newFunction(lua_CFunction f)
{
	return LuaObjectImpl::create(this,f);
}

LuaObject LuaState::newFunction(LuaCFunction f)
{
	return LuaObjectImpl::create(this,f);
}


LuaTable LuaState::newTable()
{
	return LuaObjectImpl::createTable(this);
}

LuaTable LuaState::newLib(const LuaReg funcs[])
{
	LuaTable lib=LuaObjectImpl::createTable(this);

	for (int i=0;funcs[i].funcName &&funcs[i].func ;++i)
	{
		lib.setTable(funcs[i].funcName,newFunction(funcs[i].func));
	}
	return lib;
}

LuaObject LuaState::require(const char *file)
{
	LuaFunction require=getGlobal("require");
	if (require.isValid())
	{
		return require(file);
	}
	return newNil();
}



LuaObject LuaState::doFile(const char *fileName)
{
	if(luaL_loadfile(m_ls, fileName)!=LUA_OK || lua_pcall(m_ls, 0, 1, 0)!=LUA_OK)
	{
		std::string err=lua_tostring(m_ls,-1);
		lua_pop(m_ls,1);
		throw LuaException(err.c_str());
		return luaNil;
	}
	else
	{
		return LuaObject(this,lua_gettop(m_ls));
	}
}


LuaObject LuaState::doString(const char *str)
{
	if(luaL_loadstring(m_ls, str)!=LUA_OK || lua_pcall(m_ls, 0, 1, 0)!=LUA_OK)
	{
		std::string err=lua_tostring(m_ls,-1);
		lua_pop(m_ls,1);
		throw LuaException(err.c_str());
		return luaNil;
	}
	else
	{
		return LuaObject(this,lua_gettop(m_ls));
	}
}

LuaObject LuaState::loadFile(const char *fileName)
{
	if(luaL_loadfile(m_ls, fileName)!=LUA_OK)
	{
		std::string err=lua_tostring(m_ls,-1);
		lua_pop(m_ls,1);
		throw LuaException(err.c_str());
		return luaNil;
	}
	else
		return LuaObject(this,lua_gettop(m_ls));
}

LuaObject LuaState::loadString(const char *str)
{
	if(luaL_loadstring(m_ls, str)!=LUA_OK)
	{
		std::string err=lua_tostring(m_ls,-1);
		lua_pop(m_ls,1);
		throw LuaException(err.c_str());
		return luaNil;
	}
	else
		return LuaObject(this,lua_gettop(m_ls));
}


void LuaState::error(const char* fmt,...)
{
	va_list argp;
	va_start(argp, fmt);
	luaL_where(m_ls, 1);
	lua_pushvfstring(m_ls, fmt, argp);
	va_end(argp);
	lua_concat(m_ls, 2);
	lua_error(m_ls);
}



void LuaState::enumStack()
{
	lua_State *L=m_ls;
	int top = lua_gettop(L);

	printf("Stack:%d<************************\n",top);

	for(int i=1; i<=lua_gettop(L); ++i)
	{
		switch(lua_type(L, i))
		{
		case LUA_TNIL:
			printf( "\t%s", lua_typename(L, lua_type(L, i)));
			break;
		case LUA_TBOOLEAN:
			printf("\t%s	%s", lua_typename(L, lua_type(L, i)), lua_toboolean(L, i)?"true":"false");
			break;
		case LUA_TLIGHTUSERDATA:
			printf("\t%s	0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
			break;
		case LUA_TNUMBER:
			printf("\t%s	%f", lua_typename(L, lua_type(L, i)), lua_tonumber(L, i));
			break;
		case LUA_TSTRING:
			printf("\t%s	%s", lua_typename(L, lua_type(L, i)), lua_tostring(L, i));
			break;
		case LUA_TTABLE:
			printf("\t%s	0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
			break;
		case LUA_TFUNCTION:
			printf("\t%s()	0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
			break;
		case LUA_TUSERDATA:
			printf("\t%s	0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
			break;
		case LUA_TTHREAD:
			printf("\t%s", lua_typename(L, lua_type(L, i)));
			break;
		}
	}

	printf("\n**************************>Stack\n");
}

////////////////////////////////////////////////////////////////////////////////////////


LuaFuncState::LuaFuncState(lua_State* L,bool bOwner)
	:LuaState(L),m_owner(bOwner)
{
	m_argCount=lua_gettop(L);
	if (m_argCount>LUA_MAX_ARG_COUNT)
		m_argCount=LUA_MAX_ARG_COUNT;

	for (int i=0;i<m_argCount;++i)
	{
		m_args[i]=LuaObjectImpl::createFromIndex(this,i+1);
	}
}

LuaObject LuaFuncState::arg(int count)
{
	if(count>=m_argCount || count<0)
		return LuaObject();
	else
		return m_args[count];
}

int LuaFuncState::argNum()
{
	return m_argCount;
}

void LuaFuncState::error(const char* fmt,...)
{
	char buf[1024]={0};
	va_list argp;
	va_start(argp, fmt);
	vsprintf(buf,fmt,argp);
	va_end(argp);
	throw LuaException(buf);
}




////////////////////////////////////////////////////////////////////////////////////////

//扩展的setmetatable lua的只能设置表的metatable
int luaSetMetatable(lua_State* L)
{
	if((lua_istable(L,1) || lua_isuserdata(L,1))&& lua_istable(L,2))
	{
		lua_setmetatable(L,1);
		return 1;
	}
	else
	{
		luaL_error(L,"setmetatable param error");
		return 0;
	}
}

LuaOwnerState::LuaOwnerState()
	:m_pool(0,false),LuaState(NULL),m_errorHandler(NULL)
{
	m_ls=lua_newstate(luaAlloc,&m_pool);
	luaL_openlibs(m_ls);//初始化库

	lua_pushcfunction(m_ls,luaSetMetatable);
	lua_setglobal(m_ls,"setmetatableex");

	lua_settop(m_ls,0);//清空栈
}

LuaOwnerState::~LuaOwnerState()
{
	lua_close(m_ls);
	m_ls=NULL;
}

void * LuaOwnerState::luaAlloc(void *ud, void *ptr, size_t, size_t nsize)
{
	MemPool *memalloc=(MemPool*)ud;

	if (nsize == 0) 
	{
		if(ptr)
			memalloc->free(ptr);
		return NULL;
	}
	else
	{  
		if(ptr)
			return memalloc->realloc(ptr,nsize);
		else
			return memalloc->malloc(nsize);
	}	
}


void LuaOwnerState::error(const char* fmt,...)
{
	char buf[1024]={0};
	va_list argp;
	va_start(argp, fmt);
	vsprintf(buf,fmt,argp);
	va_end(argp);

	if (m_errorHandler)
	{
		m_errorHandler(buf);
	}
	else
	{
		printf(buf);
	}
}

///////////////////////////////////////////////////////////////

TlsValue LuaAutoState::s_tlsValue;

LuaAutoState::LuaAutoState()
{
	s_tlsValue.set(this);
}

LuaAutoState::~LuaAutoState()
{
	s_tlsValue.set(NULL);
}

LuaAutoState* LuaAutoState::current()
{
	return (LuaAutoState*)(s_tlsValue.get());
}