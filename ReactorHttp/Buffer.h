#pragma once

struct Buffer
{
    //指向内存的指针
    char* data;
    //buffer大小,就是data这个char数组的大小
    int capacity;
    int readPos; //读指针位置
    int writePos;//写指针位置
};

//初始化
struct Buffer* bufferInit(int size);
//用完后释放内存
void bufferDestory(struct Buffer* buf);
//扩容
void bufferExtendRoom(struct Buffer* buffer, int size);
//得到剩余的可写的内存容量,只考虑writePos到最后的可写位置，不考虑最前端可写
int bufferWriteableSize(struct Buffer* buffer);
//得到剩余的可读的内存容量
int bufferReadableSize(struct Buffer* buffer);
//写内存 1.直接写 2.接收套接字数据
int bufferAppendData(struct Buffer* buffer, const char* data, int size);
//对上一个函数封装了一下
int bufferAppendString(struct Buffer* buffer, const char* data);
int bufferSocketRead(struct Buffer* buffer, int fd);
// 根据\r\n取出一行，找到其在数据块中的位置，返回该位置
char* bufferFindCRLF(struct Buffer* buffer);
// 发送数据
int bufferSendData(struct Buffer* buffer, int socket);