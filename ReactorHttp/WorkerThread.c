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
    pthread_cond_signal(&thread->cond); //通知主线程解除阻塞
    eventLoopRun(thread->evLoop);
    return NULL;
}

void wokerThreadRun(struct WorkerThread* thread)
{
    //创建子线程
    pthread_create(&thread->threadID, NULL, subThreadRunning, thread);
    //由于不知道子线程是否将EventLoop成功实例化，所欲需要阻塞主线程
    //阻塞主线程，让当前函数不会直接结束
    pthread_mutex_lock(&thread->mutex);
    while (thread->evLoop == NULL) // 主线程访问的是子线程的evLoop，属于访问共享资源，所以需要同步
    {//创建EventLoop可能需要一定时间，所以需要while循环一直监控，如果使用if判断，可能还是为NULL
       //此处除了条件变量，也可以使用信号量，这二者都可以起到阻塞线程的作用
        pthread_cond_wait(&thread->cond, &thread->mutex);//被条件变量阻塞
    }
    pthread_mutex_unlock(&thread->mutex);
}