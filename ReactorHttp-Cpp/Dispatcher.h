#pragma once
#include "Channel.h"
#include "EventLoop.h"
#include <string>
using namespace std;

struct EventLoop; // 先声明
class Dispatcher
{
public:
    Dispatcher(EventLoop* evLoop);
    virtual ~Dispatcher();
    //添加,将文件描述符添加到对应的dispatcher文件描述符检测集合中
    virtual int add();//EventLoop包含了Dispatcher和DispatcherData
    //删除
    virtual int remove();
    //修改
    virtual int modify();
    //事件监测
    virtual int dispatch(int timeout = 2); //单位：s
    inline void setChannel(Channel* channel){
        m_channel = channel;
    }
private:
    int epollCtl(int op);
protected:
    string m_name = string();
    Channel* m_channel;
    EventLoop* m_evLoop;
};
