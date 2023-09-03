#include "Dispatcher.h"
#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "EpollDispatcher.h"


EpollDispatcher::EpollDispatcher(EventLoop* evloop) : Dispatcher(evloop)
{
    this->m_epfd = epoll_create(10);
    if(m_epfd == -1)
    {
        perror("epoll_create");
        exit(0);    
    }

    this->m_events = new struct epoll_event[m_maxNode];
    m_name = "Epoll";
}

EpollDispatcher::~EpollDispatcher()
{
    close(m_epfd);
    delete []m_events;
}


//add/remove/modify三个函数操作相似，较为冗余，单独封装一个函数
int EpollDispatcher::epollCtl(int op)
{
    
    struct epoll_event ev;
    ev.data.fd = this-> m_channel->getSocket();
    int events = 0; 
    //不能使用if-else,需要使用两个if,可能既有读事件,也有写事件
    if (this->m_channel->getEvent() & (int)FDEvent::ReadEvent)
    {
        events |= EPOLLIN;
    }
    if(this->m_channel->getEvent() & (int)FDEvent::WriteEvent)
    {
        events |= EPOLLOUT;
    }
    
    ev.events = events;
    // int ret = epoll_ctl(data->epfd, EPOLL_CTL_ADD, channel->fd, &ev);
    int ret = epoll_ctl(data->epfd, op, channel->fd, &ev);

    return ret;
}


int EpollDispatcher::add()
{
    int ret = epollCtl(EPOLL_CTL_ADD);
    if(ret == -1)
    {   
        perror("epoll_ctl_add");
        exit(0);
    }
    return ret;
}

int EpollDispatcher::remove()
{
    int ret = epollCtl(EPOLL_CTL_DEL);
    if(ret == -1)
    {   
        perror("epoll_ctl_remove");
        exit(0);
    }
    // 通过 channel 释放对应的 TcpConnection 资源
    m_channel->destoryCallback(const_cast<void*>(m_channel->getArg()));
    
    return ret;
}

int EpollDispatcher::modify()
{
    int ret = epollCtl(EPOLL_CTL_MOD);
    if(ret == -1)
    {   
        perror("epoll_ctl_modify");
        exit(0);
    }
    return ret;
}

int EpollDispatcher::dispatch(int timeout)
{
    int count = epool_wait(m_epfd, m_events, m_maxNode, timeout*1000);
    for(int i = 0; i < count; ++i)
    {
        int events = m_events[i].events;
        int fd = m_events[i].data.fd;
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
