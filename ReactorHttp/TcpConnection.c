#include "TcpConnection.h"
#include "HttpRequest.h"
#include <stdlib.h>
#include <stdio.h>
#include "Log.h"


int processRead(void* arg)
{
    struct TcpConnection* conn = (struct TcpConnection*)arg;
    //接收数据
    int count = bufferSocketRead(conn->readBuf, conn->channel->fd);
    if(count > 0)
    {
        //接收到了http请求， 解析http请求
        int socket = conn->channel->fd;
        // 在接收到客户端请求后，才新增监听写事件
        // 只要有内存可写，写事件就会被触发
        // 但是在执行processRead函数时，写事件并不会被触发，因为此时还没有检测
        // 文件描述符集合，eventLoop并不知道写事件被激活了。
#ifdef MSG_SEND_AUTO //下面这两句代码被注释后，下面的processWrite函数就根本不会被调用了
        writeEventEnable(conn->channel, true);
        eventLoopAddTask(conn->evLoop, conn->channel, MODIFY);
#endif
        bool flag = parseHttpRequest(conn->request, conn->readBuf, conn->response, conn->writeBuf, socket);
        if (!flag)
        {
            // 解析失败，回复一个简单的html
            char* errMsg = "Http/1.1 400 Bad Request\r\n\r\n";
            bufferAppendString(conn->writeBuf, errMsg);
        }      
    }
    else
    {
#ifdef MSG_SEND_AUTO
        // 断开连接
        eventLoopAddTask(conn->evLoop, conn->channel, DELETE);
#endif
    }
#ifndef MSG_SEND_AUTO
    // 断开连接
    eventLoopAddTask(conn->evLoop, conn->channel, DELETE);
#endif

    return 0;
}

int processWrite(void* arg)
{
    struct TcpConnection* conn = (struct TcpConnection*)arg;
    // 发送数据
    int count = bufferSendData(conn->writeBuf, conn->channel->fd);
    if(count > 0)
    {
        // 步骤1与步骤2也可以不加，根据业务需要来写
        // 1. 不再检测写事件 -- 修改channel中保存的事件
        writeEventEnable(conn->channel, false);
        // 2. 修改dispatcher检测的集合 -- 添加任务节点
        eventLoopAddTask(conn->evLoop, conn->channel, MODIFY);
        // 3. 删除这个节点
        eventLoopAddTask(conn->evLoop, conn->channel, DELETE);
    }
    return 0;
}

struct TcpConnection* tcpConnectionInit(int fd, struct EventLoop* evLoop)
{
    struct TcpConnection* conn = (struct TcpConnection*)malloc(sizeof(struct TcpConnection));
    conn->evLoop = evLoop;
    conn->readBuf = bufferInit(10240);
    conn->writeBuf = bufferInit(10240);
    // http
    conn->request = httpRequestInit();
    conn->response = httpResponseInit();
    sprintf(conn->name, "Connection-%d", fd);
    conn->channel = channelInit(fd, ReadEvent, processRead, processWrite, tcpConnectionDestroy, conn);
    eventLoopAddTask(evLoop, conn->channel, ADD);

    return conn;
}

int tcpConnectionDestroy(void* arg)
{
    struct TcpConnection* conn = (struct TcpConnection*)arg;
    if(conn != NULL)
    {
        if(conn->readBuf && bufferReadableSize(conn->readBuf) == 0 &&
           conn->writeBuf && bufferReadableSize(conn->writeBuf) == 0)
        {
            destroyChannel(conn->evLoop, conn->channel);
            bufferDestroy(conn->readBuf);
            bufferDestroy(conn->writeBuf);
            httpRequestDestroy(conn->request);
            httpResponseDestroy(conn->response);
            free(conn);
        }
    }

    return 0;
}