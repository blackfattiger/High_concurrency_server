#pragma once
#include "EventLoop.h"
#include "ThreadPool.h"

struct Listener
{
    int lfd;
    unsigned short port;
};


struct TcpServer
{
    int threadNum; //用于定义线程池中子线程的个数
    struct EventLoop* mainLoop;
    struct ThreadPool* threadPool;
    struct Listener* listener;
};

// 初始化
struct TcpServer* tcpServerInit(unsigned short port, int threadNum);
// 初始化监听
struct Listener* listenerInit(unsigned short port);
// 启动服务器
void tcpServerRun(struct TcpServer* server);