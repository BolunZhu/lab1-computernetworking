#define TCP
#include "stdafx.h"
#include <stdio.h>

#include <list>
#include <algorithm>
#include <string.h>

#include <winsock.h>
// #include <winsock2.h>
//对于Winsock 2, include <winsock2.h>

#define MAXCONN 1//for single thread
#define BUFLEN 255


using namespace std;

int main(int argc, char const *argv[])
{
    //1
    WSADATA wsaData;
    int nRC;
    //2
    SOCKET srvSock;
    //3
    sockaddr_in srvAddr,clientAddr;
    //5
    u_long uNonBlock;
    int nAddrLen = sizeof(sockaddr);
    char sendBuf[BUFLEN],recvBuf[BUFLEN];
    ListCONN conList;		//保存所有有效的会话SOCKET
    ListConErr conErrList;	//保存所有失效的会话SOCKET
    FD_SET rfds,wfds;
    

    // 1.initialize winsock using startup
    nRc = WSAStartup(0x0101, & wsaData);		
    if(nRc)
    { 
    //Winsock初始化错误
    printf("Server Winsock initialize error! when startup return nRc==0\n");
    return 1;
    }
    if(wsaData.wVersion != 0x0101)
    {
    //版本支持不够
    //报告错误给用户，清除Winsock，返回
    printf("Server winsock version error!\n ")
    WSACleanup();
    return 1; 
    }
    printf("1.Sever winsock init OK!\n");
    
    //2. socket TCP/UDP
    #ifdef TCP
    srvSock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    #ifndef
    srvSock = socket(AF_INET,SOCK_DGRAM,IPPROTO_TCP);
    #endif
    
    if(srvSock == INVALID_SOCKET)
    {
    printf("Server create socket error!\n");
    WSACleanup();
    return 2;
    }
    #ifdef TCP
    printf("2.Server TCP socket create OK!\n");
    #ifndef
    printf("2.Server UDP socket create OK!\n");
    #endif

    //3.bind socket to port 5050
    printf("Please input your Server IP addr(example: 144.133.122.111)\n");
    char ipaddr[strlen(144.133.122.111)+1];
    scanf("%s",ipaddr);
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(5050);
    srvAddr.sin_addr.S_un.S_addr = inet_addr(ipaddr);
    // srvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    nRC=bind(srvSock,(LPSOCKADDR)&srvAddr,sizeof(srvAddr));
    if(nRC == SOCKET_ERROR)
    {
    printf("Server socket bind error!\n");
    closesocket(srvSock);
    WSACleanup();
    return 3;
    }
    printf("3.Server socket bind OK!\n");

    //4.listen for connect
    nRC = listen(srvSock,MAXCONN);
    if(nRC == SOCKET_ERROR)
    {
    printf("Server socket listen error! Too much connect!\n");
    closesocket(srvSock);
    WSACleanup();
    return 4;
    }
    printf("4.Sever socket linsten begin OK!\n");

    //5.accept
    int nNumConns = 0;	//当前请求连接数
    SOCKET sConns[MAXCONN];	//会话SOCKET数组
    sockaddr ConnAddrs[MAXCONN];//请求连接的客户端地址
    char inputstr[BUFLEN];//用于填入http报文的输入缓冲区
    int inputstr_actul_num=0;//用于作为recv的返回值 表示实际读取的内容大小
    while( nNumConns < MAXCONN){
    //每当收到客户端连接请求，创建新的会话SOCKET，保存在/	//sConns数组中
    //客户端地址保存在ConnAddrs数组中
    sConns[nNumConns] = accept(srvSock, 
    ConnAddrs[nNumConns], &nAddrLen);
    if(sConns[nNumConns] == INVALID_SOCKET)
    {
        printf("Cannot creat new socket accepted socket When accepted\n");
        //创建会话SOCKET出错处理
    }
    else
    {
        //创建会话SOCKET成功，启动新的线程与客户端会话
        printf("5.accept and create new socket OK!\n");
        inputstr_actul_num=0;
        inputstr_actul_num=recv(sConns[nNumConns],inputstr,BUFLEN,0);
        //这里就不对报文大小做判断力，原因有二
        // 1.一般的http报文不会太大 2.即使报文过大也可以通过http解析时发现不完整然后再次调用recv得到
        if (inputstr_actul_num > BUFLEN) {
            /* code */
        }
        
    
        //当前请求连接数+1
        nNumConns ++;
    }
    }


    return 0;
}




