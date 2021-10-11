//
// Created by jyg on 2021/7/26.
//

#ifndef NEWPROCON_MANAGE_HPP
#define NEWPROCON_MANAGE_HPP

#include <iostream>
#include <functional>
#include "signal.h"
#include <list>
#include <mutex>
#include <thread>

#include <boost/thread/pthread/shared_mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/thread/mutex.hpp>

template<class T>
class manage : private boost::noncopyable
{
    typedef boost::shared_lock<boost::mutex> TReadLock;
    typedef boost::unique_lock<boost::mutex> TWriteLock;
public:
    manage(size_t maxsize):m_itembuffermaxsize(maxsize),_buffersize(0){}
    //manage() : ItemBufferSize(20){}
    ~manage(){}

    bool manage_enqueue(const T &item)     ///管理进队,这里也加一个状态
    {
        boost::unique_lock<boost::mutex> lock(manage::m_mtx);  ///函数执行完自动进行解锁
        ///仓库判断容量是否已经装满,将while改为if
        if (m_strlist.size() == m_itembuffermaxsize)
        {
            std::cout << "缓冲区已满，请等待" << std::endl;
            //修改：不能一直阻塞
            not_full.wait_for(lock,boost::chrono::seconds(3)) == boost::cv_status::timeout;
            if(m_strlist.size() == m_itembuffermaxsize)   ///再次进行判断缓冲区是否为满
            {
                std::cout << "缓冲区已满，存储失败!" << std::endl;
                return false;
            }
            ///这里做一个修改，表示查看退出状态
        }
        ///存储生产出来的产品
        m_strlist.push_back(item);
        _buffersize++;
        not_empty.notify_all();              ///通知消费者，缓冲区不为空的特点
        return true;
    }

    bool manage_dequeue(T &data)                      ///管理出队
    {

        boost::unique_lock<boost::mutex> lock(manage::m_mtx);
        if (m_strlist.empty())
        {
            std::cout << "缓冲区为空，等待产品生产" << std::endl;
            not_empty.wait_for(lock,boost::chrono::seconds(3)) == boost::cv_status::timeout;     ///这里应该释放锁
            if(m_strlist.empty())       ///这里进行二次判断，是否为空
            {
                std::cout << "产品取出失败 ！" << std::endl;
                return false;
            }
        }
        data = m_strlist.front();
        m_strlist.pop_front();
        _buffersize--;
        not_full.notify_all();

        return true;                  ///返回出库的那个商品,进行与消费的那个通道

    }

//    ///也要进行加锁
//    void clear()
//    {
//        boost::unique_lock<boost::mutex> lock(manage::mtx);
//        if(m_strlist.empty())              ///empty()返回1则代表仓库内数据为空
//        {
//            return ;
//        }else
//        {
//            std::list<int>::iterator it;
//            for(it = m_strlist.begin(); it != m_strlist.end(); it++)
//            {
//                m_strlist.erase(it++);
//            }
//        }
//    }

    size_t  size()
    {
        TReadLock locker(m_mtx);
        return _buffersize;
    }
    bool empty()
    {
        TReadLock locker(m_mtx);
        return 0 == _buffersize;
    }

    void destroy()
    {
        delete this;
    }

private:
    boost::mutex m_mtx;   //互斥量
    boost::condition_variable not_empty;   //不为空的条件变量，反馈给消费者可以进行取数据
    boost::condition_variable not_full;    //不为满的条件变量，反馈给消费者进行生产数据
    std::list<T> m_strlist;              //仓库
    size_t m_itembuffermaxsize;
    size_t _buffersize;
};


#endif //NEWPROCON_MANAGE_HPP
