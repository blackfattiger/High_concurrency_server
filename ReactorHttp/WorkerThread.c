#include "WorkerThread.h"
#include <stdio.h>

int workerThreadInit(struct WorkerThread* thread, int index)
{
    thread->evLoop = NULL;
    thread->threadID = 0;
    sprintf(thread->name, "SubThread-%d", index);
    //下面两个函数调用第二个参数指定为空，表示使用互斥锁、条件变量的默认属性
    pthread_mutex_init(&thread->mutex, NULL);
    pthread_cond_init(&thread->cond, NULL);

    return 0;
}

//子线程的回调函数
void* subThreadRunning(void* arg)
{
    struct WorkerThread* thread = (struct WorkerThread*)arg;
    pthread_mutex_lock(&thread->mutex);
    thread->evLoop = eventLoopInitEx(thread->name);
    pthread_mutex_unlock(&thread->mutex);
    pthread_cond_signal(&thread->cond);
    eventLoopRun(thread->evLoop);
    return NULL;
}

void wokerThreadRun(struct WorkerThread* thread)
{
    //创建子线程
    pthread_create(&thread->threadID, NULL, subThreadRunning, thread);

}