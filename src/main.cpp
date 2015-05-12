
#include "gtest/gtest.h"
#include "Luaobj.h"

LuaEngine L;

TEST(LuaObj,TestInit)
{
	ASSERT_EQ(&L,LuaManager::instance()->current())<<"register current thread faield";
	ASSERT_EQ(L.getTop(),0);

	{
		LuaObject obj(&L);
		ASSERT_EQ(L.getTop(),1);
	}
	ASSERT_EQ(L.getTop(),0);
}

TEST(LuaObj,TestValue)
{
	LuaObject nil=L.newNil();
	EXPECT_TRUE(nil.isNil());

	for (lua_Number num=-10.0;num<10.0;++num)
	{
		LuaObject lnumber=L.newNumber(num);
		EXPECT_TRUE(num==lnumber.toNumber());
	}

	for (lua_Integer num=-10;num<10;++num)
	{
		LuaObject lnumber=L.newInt(num);
		EXPECT_TRUE(num==lnumber.toInt());
	}

	LuaObject ltrue=L.newBool(true);
	EXPECT_TRUE(true==ltrue.toBool());

	LuaObject lfalse=L.newBool(false);
	EXPECT_TRUE(false==lfalse.toBool());

	int data=0x2312321;
	LuaObject ldata=L.newData(&data,sizeof(data));
	EXPECT_TRUE(data==*(int*)ldata.toData() && ldata.objLen()==sizeof(data));

	void *p=(void*)0x32432;
	LuaObject lptr=L.newPtr(p);
	EXPECT_TRUE(p==lptr.toData());

	const char* str="hello world";
	LuaObject lstr=L.newString(str);
	EXPECT_TRUE(strcmp(lstr.toString(),str)==0);
}


TEST(LuaObj,TestGlobalValue)
{
	const char* str="hello world";
	L.setGlobal("testGlobal.test",L.newString(str));
	LuaObject lstr=L.getGlobal("testGlobal.test");

	EXPECT_TRUE(strcmp(lstr.toString(),str)==0);
}


int CppAddFunction (LuaState& L,LuaTable& arg)
{
	return L.lreturn(arg[1].toInt()+arg[2].toInt());
}

TEST(LuaObj,TestCallFunction)
{
	LuaFunction lAddFunc=L.newFunction(CppAddFunction);
	
	EXPECT_EQ(lAddFunc(1,2).toInt(),3);
	EXPECT_EQ(lAddFunc(1000,2000).toInt(),3000);
	EXPECT_EQ(lAddFunc(-100,200).toInt(),100);
}

TEST(LuaObj,TestCallLuaFunction)
{
	const char* luaScript="function add(a,b) return a+b end";
	L.doString(luaScript);

	LuaFunction lAddFunc=L.getGlobal("add");
	
	EXPECT_EQ(lAddFunc(1,2).toInt(),3);
	EXPECT_EQ(lAddFunc(1000,2000).toInt(),3000);
	EXPECT_EQ(lAddFunc(-100,200).toInt(),100);
}


int addToCount=1000000;

TEST(LuaObj,TestLuaPerformance)
{

	try{
		const char* luaScript="function addTo(num)local all=0;for i=1,num do all=all+i; end\nreturn all;end";
		L.doString(luaScript);

		LuaFunction lAddToFunc=L.getGlobal("addTo");

		LuaObject value=lAddToFunc(addToCount);
		lua_Integer i=value.toInt();
		printf("luaAddTo %d value %d",addToCount,i);
	}
	catch(LuaException e)
	{
		printf("%s",e.what());
	}
	
}

TEST(LuaObj,TestCppPerformance)
{
	int all=0;
	for (int i=1;i<=addToCount;++i)
	{
		all+=i;
	}

	printf("CppAddTo %d value %d",addToCount,all);
}

TEST(LuaObj,TestGetField)
{
	L.doString("a={b={c=123}}");

	LuaObject obj=L.getField("a.b.c");
	EXPECT_EQ(obj.toInt(),123);
}



int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	int rtn= RUN_ALL_TESTS();

	getchar();
	return rtn;
}






















