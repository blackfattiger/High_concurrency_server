#pragma once
#include <stdbool.h>
#include "Dispatcher.h"
#include "ChannelMap.h"
#include <pthread.h>

extern struct Dispatcher EpollDispatcher;
extern struct Dispatcher PollDispatcher;
extern struct Dispatcher SelectDispatcher;

//处理该节点中的channel
enum ElemType{ADD, DELETE, MODIFY};
//定义任务队列的节点
struct ChannelElement
{
    int type; // 如何处理该节点中的channel
    struct Channel* channel;
    struct ChannelElement* next;
};

struct Dispatcher; // 先对Dispatcher进行声明，防止未定义的引用
struct EventLoop
{
    bool isQuit;
    struct Dispatcher* dispatcher;
    void* dispatcherData;
    //任务队列
    struct ChannelElement* head;
    struct ChannelElement* tail;
    //map
    struct ChannelMap* channelMap;
    //线程id,name,mutex 互斥锁用于保护任务队列
    pthread_t threadID;
    char threadName[32];
    pthread_mutex_t mutex;
    int socketPair[2]; //存储本地通信的fd,通过socketPair初始化
};

//初始化
struct EventLoop* eventLoopInit();//初始化主线程
struct EventLoop* eventLoopInitEx(const char* threadName);//由于C没有函数重载，所以定义一个新的函数名来初始化子线程
//启动反应堆模型,调用函数时，需要知道启动的实例是谁
int eventLoopRun(struct EventLoop* evLoop);
//处理被激活的文件fd
int eventActive(struct EventLoop* evLoop, int fd, int event);  
//添加任务到任务队列
int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type);                
//处理任务队列中的任务
int eventLoopProcessTask(struct EventLoop* evLoop);
//处理dispatcher中的节点
int eventLoopAdd(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel);
//释放channel
int destroyChannel(struct EventLoop* evLoop, struct Channel* channel);