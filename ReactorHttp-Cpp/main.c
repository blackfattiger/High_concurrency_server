#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "TcpServer.h"

int main(int argc, char* argv[]){
#ifdef 0
    if (argc < 3)
    { 
        printf("./a.out port path\n");
        return -1;
    }
    unsigned short port = atoi(argv[1]);
    // 切换服务器的工作路径
    chdir(argv[2]);
#else //在编译器调试，不需要命令行进行输入
    unsigned short port = 10000;  
    chdir("/home/passion/test");  
#endif

    // 启动服务器
    struct TcpServer* server = tcpServerInit(port, 4);
    tcpServerRun(server);

    return 0;
}