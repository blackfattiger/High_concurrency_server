#include "ThreadPool.h"
#include <assert.h>


struct ThreadPool* threadPoolInit(struct EventLoop* mainLoop, int count)
{
    struct ThreadPool* pool = (struct ThreadPool*)malloc(sizeof(struct ThreadPool));
    pool->isStart = false;
    pool->mainLoop = mainLoop;
    pool->index = 0;
    pool->threadNum = count;
    pool->wokerThreads = (struct WorkerThread*)malloc(sizeof(struct WorkerThread) * count);
    return pool;
}

void threadPoolRun(struct ThreadPool* pool)
{
    assert(pool && !pool->isStart);
    if(pool->mainLoop->threadID != pthread_self()) //判断是否为主线程
    {
        exit(0);
    }
    pool->isStart = true;

    if(pool->threadNum)
    {
        for (int i = 0; i < pool->threadNum; ++i)
        {
            workerThreadInit(&pool->wokerThreads[i], i);
            wokerThreadRun(&pool->wokerThreads[i]);
        }
    }
}

struct EventLoop* takeWorkerEventLoop(struct ThreadPool* pool)
{
    assert(pool->isStart);
    if(pool->mainLoop->threadID != pthread_self()){
        exit(0);
    }
    // 从线程池中找一个子线程, 然后取出里边的反应堆实例
    struct EventLoop* evLoop = pool->mainLoop;
    if(pool->threadNum > 0)
    {
        evLoop = pool->wokerThreads[pool->index].evLoop;
        pool->index = ++pool->index % pool->threadNum;
    }
    return evLoop;
}