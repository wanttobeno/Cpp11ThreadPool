#include "threadpool.h"
#include <iostream>

void C_Function(int num)
{
	std::this_thread::sleep_for(std::chrono::microseconds(10));
   printf("C_Function  num = %d ThreadID %d\n", num,std::this_thread::get_id());
   std::this_thread::sleep_for(std::chrono::microseconds(5));
}

struct StructWithFun 
{
	int operator()(int n) 
	{
		printf("StructWithFun(%d)  ThreadID %d\n" ,n, std::this_thread::get_id() );
		return 42;
	}
};

class ClassWithStaticFun 
{   
public: 
	//函数必须是 static 的才能使用线程池
	static int StaticMemberFun1(int n = 0) 
	{
		printf("StaticMemberFun1 %d  ThreadID %d \n", n, std::this_thread::get_id());
		return n;
	}

	static std::string StaticMemberFun2(int n, std::string str, char c) 
	{
		printf("StaticMemberFun2 %d %s %c ThreadID %d \n", n, str.c_str() ,c, std::this_thread::get_id());
		return str;
	}
};


#define  TEST_TASK_COUNT 100
void AddTaskNoWaiteFinish()
{
	printf("\n %s \n", __FUNCTION__);
	mystd::threadpool threadPool{ 50 };
	for (int i = 1; i <= TEST_TASK_COUNT; i++)
	{
		threadPool.commit(C_Function, i);
	}
	printf("commit all ThreadID idlsize = %d\n",threadPool.LdleThreadPoolSize());
}

void AddTaskWaiteFinish()
{
	printf("\n %s \n", __FUNCTION__);
	mystd::threadpool threadPool(10);
	std::vector< std::future<int> > results;
	for (int i = 1; i <= TEST_TASK_COUNT; ++i)
	{
		results.emplace_back(threadPool.commit([i] {
			// 模拟任务时间
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
			printf(" num = %d  ThreadID = %d\n", i,std::this_thread::get_id());
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
			return i*i;}));
	}

	for (auto && result : results)
		result.get();
}

void GetTaskReturnValueTest()
{
	printf("\n %s \n", __FUNCTION__);
	mystd::threadpool threadPool{ 50 };
	std::future<void> cFunFuture = threadPool.commit(C_Function, 100);
	std::future<int> structFuture = threadPool.commit(StructWithFun{}, 1111);
	ClassWithStaticFun aStaticClass;
	std::future<int> StaticFun1Future = threadPool.commit(aStaticClass.StaticMemberFun1, 9999); //IDE提示错误,但可以编译运行
	std::future<std::string> StaticFun2Future = threadPool.commit(ClassWithStaticFun::StaticMemberFun2, 9998, "mult args", 123);
	std::future<std::string> LambdaFuture = threadPool.commit([]()->std::string {
		printf("Lambda ThreadID %d \n", std::this_thread::get_id());
		return "Lambda return \n"; });

	//调用.get()获取返回值会等待线程执行完,获取返回值
	cFunFuture.get();
	structFuture.get();
	StaticFun1Future.get();
	StaticFun2Future.get();
	printf("%s \n", LambdaFuture.get().c_str());
}

int main()
{
	try 
	{
		AddTaskNoWaiteFinish();
		AddTaskWaiteFinish();
		GetTaskReturnValueTest();
	}
	catch (std::exception& e) 
	{
		printf("some unhappy happened... ThreadID  %d %s\n", std::this_thread::get_id(), e.what());
	}
	getchar();
	return 0;
}
