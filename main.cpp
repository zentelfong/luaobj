#include "luaobj/Luaobj.h"

#include "gtest/gtest.h"

LuaAutoState *L=NULL;

TEST(LuaObj,TestInit)
{
	ASSERT_EQ(L,LuaAutoState::current())<<"register current thread faield";
	ASSERT_EQ(L->getTop(),0);

	{
		LuaObject obj(L);
		ASSERT_EQ(L->getTop(),1);
	}
	ASSERT_EQ(L->getTop(),0);

	{
		LuaObject * obj1=new LuaObject(L);
		LuaObject * obj2=new LuaObject(L);
		LuaObject * obj3=new LuaObject(L);
		
		delete obj2;
		delete obj1;
		delete obj3;

	}
	ASSERT_EQ(L->getTop(),0);
}

TEST(LuaObj,TestValue)
{
	LuaObject nil=L->newNil();
	EXPECT_TRUE(nil.isNil());

	for (lua_Number num=-10.0;num<10.0;++num)
	{
		LuaObject lnumber=L->newNumber(num);
		EXPECT_TRUE(num==lnumber.toNumber());
	}

	for (lua_Integer num=-10;num<10;++num)
	{
		LuaObject lnumber=L->newInt(num);
		EXPECT_TRUE(num==lnumber.toInt());
	}

	LuaObject ltrue=L->newBool(true);
	EXPECT_TRUE(true==ltrue.toBool());

	LuaObject lfalse=L->newBool(false);
	EXPECT_TRUE(false==lfalse.toBool());

	int data=0x2312321;
	LuaObject ldata=L->newData(&data,sizeof(data));
	EXPECT_TRUE(data==*(int*)ldata.toData() && ldata.objLen()==sizeof(data));

	void *p=(void*)0x32432;
	LuaObject lptr=L->newPtr(p);
	EXPECT_TRUE(p==lptr.toData());

	const char* str="hello world";
	LuaObject lstr=L->newString(str);
	EXPECT_TRUE(strcmp(lstr.toString(),str)==0);
}


TEST(LuaObj,TestGlobalValue)
{
	const char* str="hello world";
	L->setGlobal("testGlobal.test",L->newString(str));
	LuaObject lstr=L->getGlobal("testGlobal.test");

	EXPECT_TRUE(strcmp(lstr.toString(),str)==0);
}


int CppAddFunction (LuaFuncState& L)
{
	return L.lreturn(L.arg(0).toInt()+L.arg(1).toInt());
}

TEST(LuaObj,TestCallFunction)
{
	LuaFunction lAddFunc=L->newFunction(CppAddFunction);
	
	EXPECT_EQ(lAddFunc(1,2).toInt(),3);
	EXPECT_EQ(lAddFunc(1000,2000).toInt(),3000);
	EXPECT_EQ(lAddFunc(-100,200).toInt(),100);
}

TEST(LuaObj,TestCallLuaFunction)
{
	const char* luaScript="function add(a,b) return a+b end";
	L->doString(luaScript);

	LuaFunction lAddFunc=L->getGlobal("add");
	
	EXPECT_EQ(lAddFunc(1,2).toInt(),3);
	EXPECT_EQ(lAddFunc(1000,2000).toInt(),3000);
	EXPECT_EQ(lAddFunc(-100,200).toInt(),100);
}


int addToCount=1000000;

TEST(LuaObj,TestLuaPerformance)
{

	try{
		const char* luaScript="function addTo(num)local all=0;for i=1,num do all=all+i; end\nreturn all;end";
		L->doString(luaScript);

		LuaFunction lAddToFunc=L->getGlobal("addTo");

		LuaObject value=lAddToFunc(addToCount);
		lua_Integer i=value.toInt();
		printf("luaAddTo %d value %d\n",addToCount,i);
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

	printf("CppAddTo %d value %d\n",addToCount,all);
}

TEST(LuaObj,TestGetField)
{
	L->doString("a={b={c=123}}");

	LuaObject obj=L->getField("a.b.c");
	EXPECT_EQ(obj.toInt(),123);
}


class Test
{
public:
	Test(int i)
	{

	}
	void test(const char* test)
	{
		printf(test);
	}
};


class Test2:public Test
{
public:
	Test2(LuaFuncState& L)
		:Test(L.arg(0).toInt())
	{
	}

	int test(LuaFuncState& L)
	{
		Test::test(L.arg(0).toString());
		return 0;
	}

	BEGIN_MAP_FUNC(Test2)
		DECLARE_METHOD(test),
	END_MAP_FUNC
};



int main(int argc, char **argv)
{
	LuaAutoState luaEngine;
	L=&luaEngine;

	testing::InitGoogleTest(&argc, argv);
	int rtn= RUN_ALL_TESTS();

	LuaClass<Test2>::Register(L->getLuaState(),NULL);

	L->doString("local test=Test2.new();test:test('test')");
	getchar();
	return rtn;
}






















