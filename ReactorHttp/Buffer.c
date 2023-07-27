#define _GNU_SOURCE
#include "Buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>

struct Buffer* bufferInit(int size)
{
    struct Buffer* buffer = (struct Buffer*)malloc(sizeof(struct Buffer));
    if (buffer != NULL)
    {
        buffer->data = (char*)malloc(size);
        buffer->capacity = size;
        buffer->readPos = buffer->writePos = 0;
        memset(buffer->data, 0, size);
    }
    return buffer;
}

void bufferDestory(struct Buffer* buf)
{
    if(buf != NULL)
    {
        if (buf->data != NULL)
        {
            free(buf->data);
        }
    }
    
    free(buf);
}

void bufferExtendRoom(struct Buffer* buffer, int size)
{
    //1.内存够用 - 不需要扩容
    if (bufferWriteableSize(buffer) >= size)
    {
        return;
    }
    //2.内存需要合并才够用 - 不需要扩容
    //剩余的可写的内存 + 已读的内存 > size
    else if (bufferWriteableSize(buffer) + buffer->readPos >= size)
    {
        //得到未读的内存大小
        int readabel = bufferReadableSize(buffer);
        //移动内存
        memcpy(buffer->data, buffer->data + buffer->readPos, readabel);
        //更新位置
        buffer->readPos = 0;
        buffer->writePos = readabel;
    }
    //3.内存不够用 - 扩容
    else
    {
        void* temp = realloc(buffer->data, buffer->capacity + size);
        if (temp == NULL)
        {
            return; //失败了
        }
        memset(temp + buffer->capacity, 0, size);
        //更新数据
        buffer->data = temp; //扩容后，buffer的地址可能发生变化，需要重新指定
        buffer->capacity += size;//buffer的capacity也会增加
    }
}

int bufferWriteableSize(struct Buffer* buffer)
{
    return buffer->capacity - buffer->writePos;
}
         
int bufferReadableSize(struct Buffer* buffer)
{
    return buffer->writePos - buffer->readPos;
}

int bufferAppendData(struct Buffer* buffer, const char* data, int size)
{
    if (buffer == NULL || data == NULL || size <= 0)
    {
        return -1;
    }
    // 扩容——试探性扩容，不一定真的会扩容
    bufferExtendRoom(buffer, size);
    // 数据拷贝
    memcpy(buffer->data + buffer->writePos, data, size);
    buffer->writePos += size;
    return 0;
}

int bufferAppendString(struct Buffer* buffer, const char* data)
{
    int size = strlen(data);
    int ret = bufferAppendData(buffer, data, size);
    return ret;
}

int bufferSocketRead(struct Buffer* buffer, int fd)
{
    // read/recv/readv
    struct iovec vec[2];
    //初始化数组元素
    //第一块数组使用buffer的内存
    int writeable = bufferWriteableSize(buffer);
    vec[0].iov_base = buffer->data + buffer->writePos;
    vec[0].iov_len =  writeablel;
    //第二块数组就需要申请堆内存了
    char* tempbuf = (char*)malloc(40960);
    vec[1].iov_base = tempbuf;
    vec[1].iov_len =  40960;

    int result = redav(fd, vec, 2);
    if(result == -1)
    {
        return -1;
    }
    else if(result <= writeable)
    {
        buffer->writePos += result;
    }
    else
    {
        //此时肯定要用到第二块申请的堆内存，数据又不可能一直存放在堆内存中，需要将数据取出来
        buffer->writePos = buffer->capacity;
        bufferAppendData(buffer, tempbuf, resul - writeable);
    }
    
    //释放申请的堆内存
    free(tempbuf);
    return result;
}

