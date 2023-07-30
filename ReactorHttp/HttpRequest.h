#pragma once

//请求头键值对
struct RequestHeader
{
    char* key;
    char* value;
};

// 当前的解析状态
enum HttpRequestState
{
    ParseReqLine,
    ParseReqHeaders,
    ParseReqBody,
    ParseReqDone
}

//定义http请求结构体
struct HttpRequest
{
    char* method;
    char* url;
    char* version;
    struct RequestHeader* reqHeaders;
    int reqHeadersNum; //表明reqHeaders中有多少有效的键值对
    //利用枚举类型指明 解析的http请求到了哪一个状态：请求行、请求头、空行、请求体、解析完毕
    enum HttpRequestState curState;
};

//初始化
struct HttpRequest* httpRequestInit();