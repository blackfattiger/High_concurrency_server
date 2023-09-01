#pragma once
#include <pthread.h>
#include "EventLoop.h"

//定义子线程对应的结构体
struct WorkerThread
{
    char name[24];
    pthread_t threadID; // ID
    pthread_mutex_t mutex; // 互斥锁
    pthread_cond_t cond; // 条件变量，阻塞线程
    struct EventLoop* evLoop; // 反应堆模型
};

/**
 * @brief 初始化，传入工作的线程实例的地址，需要在函数体外部进行malloc内存申请，
 *        这样不需要再操心内存的管理问题；
 *        反应堆模型中的init函数是返回了一个对应类型的结构体地址，如果是这样，
 *        就需要在函数体内部malloc一块儿内存。
 *        index参数主要为了给工作线程指定名字
 * @return int 
 */
int workerThreadInit(struct WorkerThread* thread, int index);
// 启动线程
void wokerThreadRun(struct WorkerThread* thread);