#include "Dispatcher.h"
#include <sys/select.h>
#include <stdlib.h>
#include <stdio.h>

#define Max 1024

struct SelectData
{
    fd_set readSet;
    fd_set writeSet;
};

static void* selectInit(); 
static int selectAdd(struct Channel* channel, struct EventLoop* evLoop);
static int selectRemove(struct Channel* channel, struct EventLoop* evLoop);
static int selectModify(struct Channel* channel, struct EventLoop* evLoop);
static int selectDispatch(struct EventLoop* evLoop, int timeout); //单位：s
static int selectClear(struct EventLoop* evLoop);   
static void setFdSet(struct Channel* channel, struct SelectData* data);
static void clearFdSet(struct Channel* channel, struct SelectData* data);    

struct Dispatcher SelectDispatcher = {
    selectInit,
    selectAdd,
    selectRemove,
    selectModify,
    selectDispatch,
    selectClear
};

static void* selectInit()
{
    struct SelectData* data = (struct SelectData *)malloc(sizeof(struct SelectData));
    FD_ZERO(&data->readSet);
    FD_ZERO(&data->writeSet);
    return data;
}

static void setFdSet(struct Channel* channel, struct SelectData* data)
{
    if (channel->events & ReadEvent)
    {
        FD_SET(channel->fd, &data->readSet);
    }
    if (channel->events & WriteEvent)
    {
        FD_SET(channel->fd, &data->writeSet);
    }
    return 0;
}

static void clearFdSet(struct Channel* channel, struct SelectData* data)
{
    if (channel->events & ReadEvent)
    {
        FD_CLR(channel->fd, &data->readSet);
    }
    if (channel->events & WriteEvent)
    {
        FD_CLR(channel->fd, &data->writeSet);
    }
    return 0;
}


static int selectAdd(struct Channel* channel, struct EventLoop* evLoop)
{
    struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
    if (channel->fd >= Max)
    {
        return -1;
    }
    
    setFdSet(channel, data);

    return 0;
}

static int selectRemove(struct Channel* channel, struct EventLoop* evLoop)
{
    struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
    
    clearFdSet(channel, data);
    // 通过 channel 释放对应的 TcpConnection 资源
    channel->destoryCallback(channel->arg);

    return 0;
}

static int selectModify(struct Channel* channel, struct EventLoop* evLoop)
{
    struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
    setFdSet(channel, data);
    clearFdSet(channel, data);
    return 0;
}

static int selectDispatch(struct EventLoop* evLoop, int timeout)
{
    struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
    struct timeval val;
    val.tv_sec = timeout;
    val.tv_usec = 0;
    //对原始数据进行备份
    fd_set rdtmp = data->readSet;
    fd_set wrtmp = data->writeSet;
    //select函数第二第三参数都是传入传出参数，函数会对这两个参数进行修改，如果传入原数据，原数据就会被更改
    int count = select(Max, &ratmp, & wrtmp, NULL, &val);
    if (count == -1)
    {
        perror("select");
        exit(0);
    }
    
    for(int i = 0; i < Max; ++i)
    {
        if(FD_ISSET(i, &rdtmp))
        {
            eventActive(evLoop, i, ReadEvent);
        }
        
        if(FD_ISSET(i, &wrtmp))
        {
            eventActive(evLoop, i, WriteEvent);
        } 
    }
    
    return 0;
}

static int selectClear(struct EventLoop* evLoop)
{
    struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
    free(data);
    return 0;
}