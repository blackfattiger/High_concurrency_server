#include "ChannelMap.h"
#include <stdio.h>
#include <stdlib.h>

struct ChannelMap* channelMapInit(int size){
    struct ChannelMap* map = (struct ChannelMap*)malloc(sizeof(struct ChannelMap));
    map->size = size;
    map->list = (struct ChannelMap**)malloc(size * sizeof(struct channel*));
    return map;
}

void ChannelMapClear(struct ChannelMap* map){
    if(map != NULL)
    {
        for (int i = 0; i < map->size; ++i)
        {
            if(map->list[i] != NULL){ //数组内的元素内存清空
                free(map->list[i]);
            }
        }
        free(map->list); //数组的内存清空
        map->list = NULL;
    }
    map->size = 0;
}

bool makeMapRoom(struct ChannelMap* map, int newSiz, int unitSize)
{
    if(map->size < newSiz)
    {   
        int curSize = map->size;
        //容量每次扩大为原来的一倍
        while (curSize < newSiz)
        {
            curSize *= 2;
        }
        //扩容 realloc,函数返回的是空间首地址（无论是原空间还是新申请的空间）
        struct Channel** temp = realloc(map->list, curSize * unitSize);
        if(temp == NULL){
            return false;
        }
        map->list = temp;
        //将没有存储数据的空间清0
        memset(&map->list[map->size], 0, (curSize - map->size) * unitSize);
        map->size = curSize;
    }

    return true;
}