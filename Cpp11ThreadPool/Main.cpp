#include "threadpool.h"
#include <iostream>
#include <windows.h>


void C_Function(int slp)
{
	if (slp>0) {
		printf("C_Function sleep %d ThreadID %d \n",slp, std::this_thread::get_id());
		std::this_thread::sleep_for(std::chrono::milliseconds(slp));
		//Sleep(slp );
	}
	else
		printf("  C_Function  ThreadID %d\n", std::this_thread::get_id());
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
{    //函数必须是 static 的才能使用线程池
public:
	static int StaticMemberFun1(int n = 0) 
	{
		std::cout << n << "  StaticMemberFun1  ThreadID " << std::this_thread::get_id() << std::endl;
		return n;
	}

	static std::string StaticMemberFun2(int n, std::string str, char c) 
	{
		std::cout << n << "  StaticMemberFun2  "<< str.c_str() <<"  " << (int)c <<" ThreadID " << std::this_thread::get_id() << std::endl;
		return str;
	}
};

int main()
{
	try 
	{
		mystd::threadpool executor{ 50 };
		ClassWithStaticFun a;
		std::future<void> ff = executor.commit(C_Function, 0);
		std::future<int> fg = executor.commit(StructWithFun{}, 1111);
		std::future<int> gg = executor.commit(a.StaticMemberFun1, 9999); //IDE提示错误,但可以编译运行
		std::future<std::string> gh = executor.commit(ClassWithStaticFun::StaticMemberFun2, 9998, "mult args", 123);
		std::future<std::string> fh = executor.commit([]()->std::string { std::cout << "Lambda ThreadID " << std::this_thread::get_id() << std::endl; return "Lambda return \n"; });


		std::this_thread::sleep_for(std::chrono::microseconds(900));

		for (int i = 0; i < 50; i++) 
		{
			executor.commit(C_Function, i * 100);
		}
		std::cout << " commit all ThreadID " << std::this_thread::get_id() << " idlsize=" << executor.LdleThreadPoolSize() << std::endl;

		std::cout << " sleep ThreadID " << std::this_thread::get_id() << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(3));

		//调用.get()获取返回值会等待线程执行完,获取返回值
		ff.get(); 
		std::cout << fg.get() << "  " << fh.get().c_str() << "  " << std::this_thread::get_id() << std::endl;

		std::cout << " sleep  ThreadID " << std::this_thread::get_id() << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(3));

		std::cout << " C_Function(55) ThreadID " << std::this_thread::get_id() << std::endl;
		//调用.get()获取返回值会等待线程执行完
		executor.commit(C_Function, 55).get();    

		std::cout << "end... " << std::this_thread::get_id() << std::endl;

		mystd::threadpool pool(4);
		std::vector< std::future<int> > results;

		for (int i = 0; i < 8; ++i) 
		{
			results.emplace_back(
				pool.commit([i] {
				std::cout << "hello " << i << std::endl;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				std::cout << "world " << i << std::endl;
				return i*i;
			})
				);
		}
		std::cout << " commit all2 ThreadID  " << std::this_thread::get_id() << std::endl;

		for (auto && result : results)
			std::cout << result.get() << ' ';
		std::cout << std::endl;
	}
	catch (std::exception& e) 
	{
		std::cout << "some unhappy happened... ThreadID  " << std::this_thread::get_id() << e.what() << std::endl;
	}
	getchar();
	return 0;
}
