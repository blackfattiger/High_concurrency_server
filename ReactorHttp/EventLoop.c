#include "EventLoop.h"
#include <assert.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct EventLoop* eventLoopInit()
{
    return eventLoopInitEx(NULL);
}

//写数据,用于主线程唤醒子线程
void taskWakeup(struct EventLoop* evLoop)
{
    const char* msg = "醒得了，别睡了！！！";
    write(evLoop->socketPair[1], msg, sizeof(msg));
}
//读数据
int readLocalMessage(void* arg)
{
    struct EventLoop* evLoop = (struct EventLoop*)arg;
    char buf[256];
    /*目的并不是读数据，而是让socketPair[1]触发一次读事件,
    触发了读事件之后，检测该文件描述符得底层模型->poll、select、epoll_wait就解除阻塞了
    */
    read(evLoop->socketPair[1], buf, sizeof(buf));

    return 0;
}

struct EventLoop* eventLoopInitEx(const char* threadName)
{
    struct EventLoop* evLoop = (struct EventLoop*)malloc(sizeof(struct EventLoop));
    evLoop->isQuit = false;
    evLoop->threadID = pthread_self();
    pthread_mutex_init(&evLoop->mutex, NULL);
    strcpy(evLoop->threadName, threadName == NULL ? "MainThread" : threadName);
    evLoop->dispatcher = &EpollDispatcher; // 这里可以随意选择epoll、poll、select
    evLoop->dispatcherData = evLoop->dispatcher->init();
    //链表
    evLoop->head = evLoop->tail = NULL;
    //map
    evLoop->channelMap = channelMapInit(128);
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, evLoop->socketPair);
    if (ret = -1)
    {
        perror("socketpair");
        exit(0);
    }
    //指定规则：evLoop->socketPair[0]发送数据， evLoop->socketPair[1]接收数据
    struct Channel* channel = channelInit(evLoop->socketPair[1], ReadEvent, 
        readLocalMessage, NULL, NULL, evLoop);
    // channel添加到任务队列
    eventLoopAddTask(evLoop, channel, ADD);
    /*光添加到任务队列还不行，还需要遍历任务队列，从任务队列中取出每一个节点，
    根据节点里面的类型，对节点做操作->channel中的文件描述符最终会被添加到
    dispatcher对应的检测集合里面*/

    return evLoop;
}

int eventLoopRun(struct EventLoop* evLoop)
{
    assert(evLoop != NULL);
    //取出事件分发和检测模型
    struct Dispatcher* dispatcher = evLoop->dispatcher;
    //比较线程ID是否正常,若不正常直接退出
    if (evLoop->threadID != pthread_self())
    {
        return -1;
    }
    
    
    //循环进行事件处理
    while (!evLoop->isQuit)
    {
        //难点：dispatch是函数指针，其指向的任务函数是动态的！
        //回调函数的优点就是可以绑定多个不同的函数，这里的dispatch就指向了epoll、poll、select的三个不同的监听函数
        dispatcher->dispatch(evLoop, 2); //超时时长 2s
        /*如果是子线程自己往任务队列添加任务，子线程肯定没有阻塞，并且可以自行完成
        处理任务队列任务的功能；
        如果是主线程发送消息让子线程解除阻塞，若子线程阻塞，子线程就从dispatch解除阻塞，
        就需要在dispatch函数下面使用eventLoopProcessTask函数让子线程处理任务队列。
        */
        eventLoopProcessTask(evLoop);
    }
    
    return 0;
}

int eventActive(struct EventLoop* evLoop, int fd, int event)
{
    if(fd < 0 || evLoop == NULL){
        return -1;
    }
    //channel里面注册的回调函数调用的时机就是读事件或写事件被触发了
    //取出channel
    struct Channel* channel = evLoop->channelMap->list[fd];
    assert(channel->fd == fd);

    if(event & ReadEvent && channel->readCallback)
    {
        channel->readCallback(channel->arg);
    }
    if(event & WriteEvent && channel->writeCallback)
    {   
        channel->writeCallback(channel->arg);
    }
    return 0;
}

int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type)
{
    //任务队列可能会被多个线程访问，所以需要加锁，保护共享资源
    pthread_mutex_lock(&evLoop->mutex);
    //创建新节点，然后将该节点添加到任务队列中
    struct ChannelElement* node = (struct ChannelElement*)malloc(sizeof(struct ChannelElement));
    node->channel = channel;
    node->type = type;
    node->next = NULL;
    //判断链表是否为空
    if(evLoop->head == NULL)
    {
        evLoop->head = evLoop->tail =node;
    }
    else
    {
        evLoop->tail->next = node; //添加
        evLoop->tail = node;       //后移
    }
    pthread_mutex_unlock(&evLoop->mutex);
    //处理节点
    /*
        若当前的EventLoop反应堆属于子线程
        细节：
            1.对于链表节点的添加：可能是当前线程也可能是其他线程（主线程）
                1）修改fd的事件，当前子线程发起，当前子线程处理
                2）添加新的fd，添加任务节点的操作是由主线程发起的
            2.不能让主线程处理任务队列，需要由当前的子线程去处理
    */
    if(evLoop->threadID == pthread_self())
    {
        //当前子线程（基于子线程的角度分析）
        eventLoopProcessTask(evLoop);
    }
    else
    {
        //主线程 -- 告诉子线程处理任务队列中的任务
        //1.子线程在工作 2.子线程被阻塞了：select、poll、epoll
        /*
        子线程有可能被epoll_wait()等函数阻塞，由于监听的文件描述符列表没有读写事件，
        所以一直阻塞,此时如果需要对监听的文件描述符列表进行增删改操作，就需要使
        子线程退出阻塞状态，转而处理件描述符列表增删改的操作，处理完毕后，再继续
        进行监听动作！要唤醒阻塞的子线程，需要一个fd触发读写事件，所以可以自己
        准备一个fd，用于自己控制对文件描述符列表的激活。
        */
       taskWakeup(evLoop);
    }

    return 0;
}

int eventLoopProcessTask(struct EventLoop* evLoop)
{
    pthread_mutex_lock(&evLoop->mutex);
    //取出头节点
    struct ChannelElement* head = evLoop->head;
    while (head != NULL)
    {
        struct Channel* channel = head->channel;
        if (head->type == ADD)
        {
            //添加
            eventLoopAdd(evLoop, channel);
        }
        else if(head->type == DELETE)
        {
            //删除
            eventLoopRemove(evLoop, channel);
        }
        else if(head->type == MODIFY)
        {
            //修改
            eventLoopModify(evLoop, channel);
        }

        struct ChannelElement* tmp = evLoop->head;
        head = head->next;
        free(tmp);
    }
    evLoop->head = evLoop->tail = NULL;
    pthread_mutex_unlock(&evLoop->mutex);
    return 0;
}

/*注意区分此函数与上面AddTask函数的区别：
AddTask函数是将任务添加到EventLoop这个任务模型里面；
现在任务队列里面已经有任务了，需要处理任务队列中的任务，此处的添加是将任务节点
中的任务添加到dispatcher对应的检测集合里面。
*/
int eventLoopAdd(struct EventLoop* evLoop, struct Chan
nel* channel)
{
    int fd = channel->fd;
    struct ChannelMap* channelMap = evLoop->channelMap;
    if (fd >= channelMap->size)  
    {
        //没有足够的空间存储键值对 fd <-> channel ==>扩容,若扩容失败返回-1
        if(!makeMapRoom(channelMap, fd, sizeof(struct Channel)))
        {
            return -1;
        }
    }
    //找到fd对应的数组元素位置，并存储
    if (channelMap->list[fd] == NULL)
    {
        //下面两行即为该函数的核心代码
        channelMap->list[fd] = channel;
        evLoop->dispatcher->add(channel, evLoop);
    }
    return 0;
}

int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel)
{
    int fd = channel->fd;
    struct ChannelMap* channelMap = evLoop->channelMap;
    if (fd >= channelMap->size)  
    {
        return -1;
    }
    int ret = evLoop->dispatcher->remove(channel, evLoop);
    return ret;
}

int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel)
{
    int fd = channel->fd;
    struct ChannelMap* channelMap = evLoop->channelMap;
    if (fd >= channelMap->size || channelMap->list[fd] == NULL)  
    {
        return -1;
    }
    int ret = evLoop->dispatcher->modify(channel, evLoop);
    return ret;
}

int destroyChannel(struct EventLoop* evLoop, struct Channel* channel)
{
    //删除 channel 和 fd 的对应关系
    evLoop->channelMap[channel->fd] = NULL;
    //关闭fd
    close(channel->fd);
    //释放channel
    free(channel);
    return 0;
}