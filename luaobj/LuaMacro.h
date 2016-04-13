#pragma once

#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif
#include <assert.h>

#if defined(LUAOBJ_BUILD_AS_DLL)
	#if defined(LUA_CORE) || defined(LUA_LIB)
		#define LUAOBJ_API __declspec(dllexport)
	#else
		#define LUAOBJ_API __declspec(dllimport)
	#endif
#else
	#define LUAOBJ_API
#endif


#ifdef _WIN32
	#define TLSVAR			DWORD
	#define TLSALLOC(k)	(*(k)=TlsAlloc(), TLS_OUT_OF_INDEXES==*(k))
	#define TLSFREE(k)		(!TlsFree(k))
	#define TLSSET(k, a)	(!TlsSetValue(k, a))

	#ifdef DEBUG
		inline LPVOID CheckedTlsGetValue(DWORD idx)
		{
			LPVOID ret=TlsGetValue(idx);
			assert(S_OK==GetLastError());
			return ret;
		}
		#define TLSGET(k) CheckedTlsGetValue(k)
	#else
		#define TLSGET(k)		TlsGetValue(k)
	#endif

#else
	#define TLSVAR			pthread_key_t
	#define TLSALLOC(k)	pthread_key_create(k, 0)
	#define TLSFREE(k)		pthread_key_delete(k)
	#define TLSGET(k)		pthread_getspecific(k)
	#define TLSSET(k, a)	pthread_setspecific(k, a)
#endif