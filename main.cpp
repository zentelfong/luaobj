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
	LuaObject ldata=L->newData(sizeof(data),&data);
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
	//L->doString("a={b={c=123}}");
	L->setField("a.b.c",L->newInt(123));
	LuaObject obj=L->getField("a.b.c");
	EXPECT_EQ(obj.toInt(),123);
}

TEST(LuaObj,TestTableIterator)
{
	L->doString("a={1,2,3,4,a=5,b=6,c=7}");
	L->enumStack();
	LuaTable tab=L->getGlobal("a");
	for (LuaTable::Iterator i=tab.begin();i!=tab.end();++i)
	{
		printf("%d\t",i.value().toInt());
	}
	
	printf("\n");
	L->enumStack();
}




class Test
{
public:
	LUAOBJ_CLASS_NAME("cc.Test");

	Test(int i)
		:m_mem(i)
	{

	}
	void test(const char* test)
	{
		printf("Test%d:%s\n",m_mem,test);
	}
private:
	int m_mem;
};


class LTest:public LuaClass<Test>
{
public:
	void ctor(LuaFuncState& L)
	{
		pThis = new Test(L.arg(0).toInt());
	}

	int test(LuaFuncState& L)
	{
		pThis->test(L.arg(0).toString());
		return 0;
	}

	BEGIN_MAP_FUNC(LTest)
		DECLARE_FUNC(test),
	END_MAP_FUNC
};



class Parent
{
public:
	LUAOBJ_CLASS_NAME("cc.Parent");

	void test(const char* test)
	{
		printf("Parent:%s\n",test);
	}

	void test2(Test* test)
	{
		test->test("Parent");
	}
};

class Child:public Parent
{
public:
	LUAOBJ_CLASS_NAME("cc.Child");

	void test(const char* test)
	{
		printf("Child:%s\n",test);
	}
};



class LParent:public LuaClass<Parent>
{
public:

	int test(LuaFuncState& L)
	{
		pThis->test(L.arg(0).toString());
		return 0;
	}

	int test2(LuaFuncState& L)
	{
		Test* test=LuaClass<Test>::toC(L.arg(0));
		pThis->test2(test);
		return 0;
	}

	BEGIN_MAP_FUNC(LParent)
		DECLARE_FUNC(test),
		DECLARE_FUNC(test2),
	END_MAP_FUNC
};

class LChild :public LuaClass<Child>
{
public:

	int test2(LuaFuncState& L)
	{
		pThis->test(L.arg(0).toString());
		return 0;
	}

	void dtor()
	{
		delete pThis;
	}

	BEGIN_MAP_FUNC(LChild)
		DECLARE_FUNC(test2),
	END_MAP_FUNC
};


//æ≤Ã¨¿‡
class StaticClass
{
public:
	LUAOBJ_CLASS_NAME("cc.Static");

	static StaticClass* Instance()
	{
		static StaticClass sc;
		return &sc;
	}

	void test(const char* test)
	{
		printf("Static:%s\n",test);
	}

};

class LStaticClass :public LuaClass<StaticClass>
{
public:

	static StaticClass* instance()
	{
		return StaticClass::Instance();
	}

	int test(LuaFuncState& L)
	{
		pThis->test(L.arg(0).toString());
		return 0;
	}


	BEGIN_MAP_FUNC(LStaticClass)
		DECLARE_FUNC(test),
	END_MAP_FUNC
};





int test()
{
	LuaAutoState luaEngine;
	L=&luaEngine;
	int rtn= RUN_ALL_TESTS();

	LuaRegister<LTest>::Register(L->getLuaState());
	LuaRegister<LParent>::Register(L->getLuaState());
	LuaRegister<LChild>::Register<LParent>(L->getLuaState());

	LuaRegister<LStaticClass>::RegisterStatic(L->getLuaState());


	L->doString("local test=cc.Test.new(123);test:test('test1')");
	L->doString("local test=cc.Parent.new();test:test('test2');test:test2(cc.Test.new(222))");
	L->doString("local test=cc.Child.new();test:test('test');test:test2('test3')");

	L->doString("local s1=cc.Static.instance();local s2=cc.Static.instance();");
	//L->doString("local test1=cc.Static.instance();test1:test('ddddd')");
	return rtn;
}


int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);	
	int rtn=test();
	getchar();
	return rtn;
}






















