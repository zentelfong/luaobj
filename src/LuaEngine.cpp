#include "LuaEngine.h"

LuaManager s_LuaManager;

LuaManager::LuaManager(void)
{
	TLSALLOC(&m_key);
}


LuaManager::~LuaManager(void)
{
	 TLSFREE(m_key);
}


LuaManager* LuaManager::instance()
{
	return &s_LuaManager;
}

LuaEngine* LuaManager::current()
{
	return static_cast<LuaEngine *>(TLSGET(m_key));
}

void LuaManager::setCurrent(LuaEngine* engine)
{
	TLSSET(m_key, engine);
}




LuaEngine::LuaEngine()
{
	s_LuaManager.setCurrent(this);
}

LuaEngine::~LuaEngine()
{
	s_LuaManager.setCurrent(NULL);
}



















