#pragma once
#include <functional>

//定义函数指针
// typedef int(* handleFunc)(void* arg);
using handleFunc = int(*)(void* arg);

//定义文件描述符的读写事件
enum class FDEvent // 使用C++11的强类型枚举新特性,使用时要像调用类的静态函数一样加上作用域
{
    TimeOut = 0x01,
    ReadEvent = 0x02,
    WriteEvent = 0x04
};

class Channel
{
public:
    Channel(int fd, int events, handleFunc readFunc, handleFunc writeFunc, handleFunc destoryFunc, void* arg);
    //回调函数
    handleFunc readCallback;
    handleFunc writeCallback;
    handleFunc destoryCallback;

    //修改fd的写事件（检测or不检测）
    void writeEventEnable(bool flag);
    //判断是否需要检测文件描述符的写事件 
    bool isWriteEventEnable();
    // 取出私有成员的值 
    // 使用内联函数，内联函数不需要压栈而是直接进行代码块的替换，提高程序的执行效率，但会使用更多的内存
    inline int getEvent(){
        return m_events;
    }
    inline int getSocket(){
        return m_fd;
    }
    inline const void* getArg(){  // 返回一个只读的地址，防止数据被修改
        return m_arg;
    }
private:
    //文件描述符
    int m_fd;
    //事件
    int m_events;
    //回调函数的参数
    void* m_arg;
};

