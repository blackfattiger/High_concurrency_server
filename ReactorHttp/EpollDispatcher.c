#include "Dispatcher.h"
#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define Max 520

struct EpollData
{
    int epfd;
    struct epoll_event* events;
};


static void* epollInit(); 
static int epollAdd(struct Channel* channel, struct EventLoop* evLoop);
static int epollRemove(struct Channel* channel, struct EventLoop* evLoop);
static int epollModify(struct Channel* channel, struct EventLoop* evLoop);
static int epollDispatch(struct EventLoop* evLoop, int timeout); //单位：s
static int epollClear(struct EventLoop* evLoop);   
static int epollCtl(struct Channel* channel, struct EventLoop* evLoop, int op);    

struct Dispatcher EpollDispatcher = {
    epollInit,
    epollAdd,
    epollRemove,
    epollModify,
    epollDispatch,
    epollClear
};

static void* epollInit()
{
    struct EpollData* data = (struct EpollData *)malloc(sizeof(struct EpollData));
    data->epfd = epoll_create(10);
    if(data->epfd == -1)
    {
        perror("epoll_create");
        exit(0);    
    }
    //calloc 内存申请+初始化为0一条龙
    data->events = (struct epoll_event*)calloc(Max, sizeof(struct epoll_event));
    return data;
}

//add/remove/modify三个函数操作相似，较为冗余，单独封装一个函数
static int epollCtl(struct Channel* channel, struct EventLoop* evLoop, int op)
{
    struct EpollData* data = (struct epollData*)evLoop->dispatcherData;
    struct epoll_event ev;
    ev.data.fd = channel->fd;
    int events = 0; 
    //不能使用if-else,需要使用两个if,可能既有读事件,也有写事件
    if (channel->events & ReadEvent)
    {
        events |= EPOLLIN;
    }
    if(channel->events & WriteEvent)
    {
        events |= EPOLLOUT;
    }
    
    ev.events = events;
    // int ret = epoll_ctl(data->epfd, EPOLL_CTL_ADD, channel->fd, &ev);
    int ret = epoll_ctl(data->epfd, op, channel->fd, &ev);

    return ret;
}


static int epollAdd(struct Channel* channel, struct EventLoop* evLoop)
{
    int ret = epollCtl(channel, evLoop, EPOLL_CTL_ADD);
    if(ret == -1)
    {   
        perror("epoll_ctl_add");
        exit(0);
    }
    return ret;
}

static int epollRemove(struct Channel* channel, struct EventLoop* evLoop)
{
    int ret = epollCtl(channel, evLoop, EPOLL_CTL_DEL);
    if(ret == -1)
    {   
        perror("epoll_ctl_remove");
        exit(0);
    }
    return ret;
}

static int epollModify(struct Channel* channel, struct EventLoop* evLoop)
{
    int ret = epollCtl(channel, evLoop, EPOLL_CTL_MOD);
    if(ret == -1)
    {   
        perror("epoll_ctl_modify");
        exit(0);
    }
    return ret;
}

static int epollDispatch(struct EventLoop* evLoop, int timeout)
{
    struct EpollData* data = (struct epollData*)evLoop->dispatcherData;
    int count = epool_wait(data->epfd, data->events, Max, timeout*1000);
    for(int i = 0; i < count; ++i)
    {
        int events = data->events[i].events;
        int fd = data->events[i].data.fd;
        if(events & EPOLLERR || events & EPOLLHUP)
        {
            //对方断开了连接，删除fd
            // epollRemove(Channel, evLoop);
            continue;
        }
        if(events & EPOLLIN)
        {
            eventActive(evLoop, fd, ReadEvent);
        }
        if(events & EPOLLOUT)
        {
            eventActive(evLoop, fd, WriteEvent);
        }
    }
    return 0;
}

static int epollClear(struct EventLoop* evLoop)
{
    struct EpollData* data = (struct epollData*)evLoop->dispatcherData;
    free(data->events);
    close(data->epfd);
    free(data);
    return 0;
}       
