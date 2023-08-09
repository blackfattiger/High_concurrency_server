#pragma once
#include "Buffer.h"

// 定义状态码枚举
enum HttpStatusCode
{
    Unknown,
    OK = 200,
    MovedPermanently = 301,
    MovedTemporarily = 302,
    BadRequest = 400,
    NotFound = 404
};

// 定义响应的结构体
struct ResponseHeader
{
    char key[32];
    char value[128];
};

// 定义一个函数指针，用来组织要回复给客户端的数据块
/* 此处通过函数指针进行回调的优势就显现出来了，不需要知道具体怎么处理的，只需要
   在这里打个桩，定义一下这里需要函数指针，在之后的调用中，你知道它要走什么处理，
   你就给这个函数指针指定对应的函数，就可以完成对应的处理动作。
*/
typedef void (*responseBody)(const char* fileName, struct Buffer* sendBuf, int socket);

// 定义结构体
struct HttpResponse
{
    // 状态行：状态码，状态描述
    enum HttpStatusCode statusCode;
    char statusMsg[128];
    char fileName[128];
    // 响应头 - 键值对
    struct ResponseHeader* headers;
    int headerNum;
    // 添加函数指针变量
    responseBody sendDataFunc;
};

// 初始化
struct HttpResponse* httpResponseInit();
// 销毁
void httpResponseDestroy(struct HttpResponse* response);
// 添加响应头
void httpResponseAddHeader(struct HttpResponse* response, const char* key, const char* value);
// 组织http响应数据
void httpResponsePrepareMsg(struct HttpResponse* response, struct Buffer* sendBuf, int socket);