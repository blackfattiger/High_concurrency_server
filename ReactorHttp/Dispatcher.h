#pragma once
#include "Channel.h"
#include "EventLoop.h"

struct Dispatcher
{
    //init -- 初始化epoll,poll或者select 需要的数据块
    void* (*init)(); 
    //添加,将文件描述符添加到对应的dispatcher文件描述符检测集合中
    int (*add)(struct Channel* channel, struct EventLoop* evLoop);//EventLoop包含了Dispatcher和DispatcherData
    //删除
    int (*remove)(struct Channel* channel, struct EventLoop* evLoop);
    //修改
    int (*modify)(struct Channel* channel, struct EventLoop* evLoop);
    //事件监测
    int (*dispatch)(struct EventLoop* evLoop, int timeout); //单位：s
    //清除数据（关闭fd或者释放内存）
    int (*clear)(struct EventLoop* evLoop);
};
