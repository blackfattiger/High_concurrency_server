#include "Channel.h"
#include <stdlib.h>

Channel::Channel(int fd, int events, handleFunc readFunc, handleFunc writeFunc, handleFunc destoryFunc, void* arg)
{
    this->m_arg = arg;
    this->m_fd = fd;
    this->m_events = events;
    this->readCallback = readFunc;
    this->writeCallback = writeFunc;
    this->destoryCallback = destoryFunc;
}

void Channel::writeEventEnable(bool flag){
    if (flag)
    {
        // this->m_events |= (int)FDEvent::WriteEvent;
        this->m_events |= static_cast<int>(FDEvent::WriteEvent);
    }
    else
    {
        this->m_events = this->m_events & ~(int)FDEvent::WriteEvent;
    }
}

bool Channel::isWriteEventEnable()
{
    // if (channel->events)
    // {
    //     return true;
    // }
    // return false;
    return this->m_events & (int)FDEvent::WriteEvent; //大于0成立， 等于0不成立
}