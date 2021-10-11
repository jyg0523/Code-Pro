#include <iostream>
#include <functional>
#include "signal.h"
#include "Manage.hpp"
#include <unistd.h>
#include <hash_map>


volatile bool g_run_flag = true;

class Work
{
public:
    Work(){}
    Work(manage<int> *queue): m_queue(queue)   ///这是一个模板类，<int>表明这是一个int类型
    {

    }
    virtual ~Work(){}

    virtual bool init()
    {
        return true;
    }

    virtual void fini() = 0;

    virtual void srv() = 0;

protected:
    manage<int> *m_queue;
};

///继承 Work类
class producer : public Work
{

public:
    ///增加一个manage的对象
    producer(manage<int> *m_queue): Work(m_queue){}
    ~producer(){}
    ///生产者进行数据的生产,传递一个数量参数，超过这个参数就停止生产
    ///还有一个就是循环无线生产，传递一个信号的时候，调用某个stop函数进行线程终止
    void srv()  override ///静态函数里面没有this指针
    {
		bool backstate;
        ///如果接收到这个信号，直接就return
        while(g_run_flag)
        {
            //sleep(1);
            int num = rand() % 100;          ///随机产生一个数据
            std::cout << "Thread : " << std::this_thread::get_id() << ", run flag: " << g_run_flag << "  produced the item : " << num << std::endl;
            backstate = m_queue->manage_enqueue(num);
        }

    }

    bool init()
    {
        return true;
    }

    void fini()
    {

    }

};

///继承 Work类
class Consumer : public Work
{
public :
    Consumer(manage<int> *m_queue): Work(m_queue){}
    ~Consumer(){}

    void srv()  override
    {
        while(g_run_flag)
        {
            int item = 0;
            bool state = m_queue->manage_dequeue(item);     ///传入的是一个引用，引用内部发生变化的时候,这个变量也会改变
            if (state)   ///判断是否获取数据
            {
                //sleep(1);
                item = item % 10;                             ///消费者对这个商品进行处理
                std::cout << "Thread : " << std::this_thread::get_id() << ", run flag: " << g_run_flag << "  consumed the item : "
                          << item << std::endl;
            }else
            {
                std::cout << "仓库为空，没有获取到数据，等待....." << std::endl;
            }
        }
    }

    bool inti()
    {
        return true;
    }

    void fini()
    {

    }
};

void signal_handle(int sig)
{
	g_run_flag = false;
	std::cout << "recv exit signal..." << std::endl;
}

int main()
{
    ///对信号进行初始化
    signal(SIGINT, signal_handle);    //ctrl + c
    signal(SIGTERM, signal_handle);   //kill命令
    signal(SIGABRT, signal_handle);   //

    ///构造仓库资源
    manage<int> queue(100);
    Work *p1 = new producer(&queue);   //基类指针指向一个派生类对象
    Work *c1 = new Consumer(&queue);

    ///使用bind函数将类对象和成员函数绑定起来，启动一个线程，创建一个对象
    std::thread pro1(std::bind(&Work::srv,p1));
    std::thread pro2(std::bind(&Work::srv,p1));
    std::thread con1(std::bind(&Work::srv,c1));
    std::thread con2(std::bind(&Work::srv,c1));

    ///等待线程退出
    pro1.join();
    pro2.join();
    con1.join();
    con2.join();

    p1->fini();
    c1->fini();

    ///回收类对象
    delete p1;
    delete c1;

    ///销毁缓冲区
//    queue.destroy();
    return 0;
}



/*
#include <string>
#include <iostream>
#include <hash_map>

using namespace std;
using namespace __gnu_cxx;

//define the class
class ClassA
{
public:
    ClassA(int a):c_a(a){}
    int getvalue()const { return c_a;}
    void setvalue(int a){c_a = a;}
private:
    int c_a;
};

//1 define the hash function
struct hash_A{
    size_t operator()(const  ClassA & A)const{
        //  return  hash<int>(classA.getvalue());
        return A.getvalue();
    }
};

//2 define the equal function
struct equal_A
{
    bool operator()(const  ClassA & a1, const  ClassA & a2)const
    {
        return  a1.getvalue() == a2.getvalue();
    }
};

int main()
{
    hash_map<ClassA, string, hash_A, equal_A> hmap;
    ClassA a1(12);
    hmap[a1]="I am 12";
    ClassA a2(198877);
    hmap[a2]="I am 198877";

    std::cout << hmap[12] <<std::endl;
    hash_map<ClassA,string, hash_A, equal_A>::iterator  map;  ///定义一个迭代器
    map = hmap.find(12);
    std::cout << map->first.getvalue() << ":" << map->second << std::endl;
    cout<<hmap[a2]<<endl;
    return 0;
}

*/