
solution "LuaObj"
	location ( "build" )
	configurations { "Debug", "Release" }
	platforms {"x64", "x32"}
	
   	project "dlmalloc"
		language "C"
		kind "StaticLib"
		files { "malloc/dlmalloc.c","malloc/dlmalloc.h" }
		targetdir("build")
		defines { "_CRT_SECURE_NO_WARNINGS" }
		
		configuration "Debug"
			defines { "DEBUG" }
			flags { "Symbols", "ExtraWarnings"}

		configuration "Release"
			defines { "NDEBUG" }
			flags { "Optimize", "ExtraWarnings"}
	
   	project "gtest"
		language "C++"
		kind "StaticLib"
		includedirs { "gtest","./" }
		files { "gtest/*.h","gtest/src/gtest-all.cc" }
		targetdir("build")
		defines { "_CRT_SECURE_NO_WARNINGS" }
		
		configuration "Debug"
			defines { "DEBUG" }
			flags { "Symbols", "ExtraWarnings"}

		configuration "Release"
			defines { "NDEBUG" }
			flags { "Optimize", "ExtraWarnings"}	
	
   	project "lua"
		language "C"
		kind "StaticLib"
		includedirs { "lua" }
		files { "lua/*.h","lua/*.c" }
		removefiles {"lua/luac.c","lua/lua.c"}
		targetdir("build")
		defines { "_CRT_SECURE_NO_WARNINGS" }
		
		configuration "Debug"
			defines { "DEBUG" }
			flags { "Symbols", "ExtraWarnings"}

		configuration "Release"
			defines { "NDEBUG" }
			flags { "Optimize", "ExtraWarnings"}		
	
	
	
   	project "LuaObj"
		language "C++"
		kind "StaticLib"
		includedirs { "luaobj","./" }
		files { "luaobj/*.h","luaobj/*.cpp" }
		targetdir("build")
		defines { "_CRT_SECURE_NO_WARNINGS" }
		
		configuration "Debug"
			defines { "DEBUG" }
			flags { "Symbols", "ExtraWarnings"}

		configuration "Release"
			defines { "NDEBUG" }
			flags { "Optimize", "ExtraWarnings"}

			
	project "test"
		kind "ConsoleApp"
		language "C++"
		includedirs { "./" }
		files { "main.cpp" }
		targetdir("build")
		links { "dlmalloc","gtest","lua","LuaObj" }

		configuration "Debug"
			defines { "DEBUG" }
			flags { "Symbols", "ExtraWarnings"}

		configuration "Release"
			defines { "NDEBUG" }
			flags { "Optimize", "ExtraWarnings"}



