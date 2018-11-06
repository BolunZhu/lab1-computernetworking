//#include "stdafx.h"
#include <stdio.h>

#include <list>
#include <algorithm>
#include <string.h>

#include <winsock.h>
// #include <winsock2.h>
//对于Winsock 2, include <winsock2.h>
#include <thread>
#include <regex>
#define MAXCONN 10//for single thread
#define BUFLEN 255
static const char ERROR_HTML_PAGE[] = "HTTP/1.1 404 Not Found\r\nContent-Type: \
text/html\r\nContent-Length: 78\r\n\r\n<HTML>\r\n<BODY>\r\nSorry, the page you requested was not found.\
\r\n</BODY>\r\n</HTML>\r\n\0";

using namespace std;
//这个函数是用来将字符串发送给客户端的
// int write(const char *usrbuf, int n)
int write(SOCKET s,const char *usrbuf, int n){
	//fputs(usrbuf, stdout);
	int left = n;
	const char *bufp = usrbuf;
	int written = 0;
	int err = 0;
	while (left>0)
	{
		written = send(s, usrbuf, left, 0);
		if(written == 0)  ;//end
		if (written < 0){
            printf("write error ,Exit!\n");
            std::terminate();
        }
		left -= written;
		bufp += written;
	}
	return n - left;//left to written?
};

void http_this(SOCKET this_socket,sockaddr addr)
{
    char inputstr[BUFLEN+1];//用于填入http报文的输入缓冲区
    int inputstr_actul_num=0;//用于作为recv的返回值 表示实际读取的内容大小
    std::stringstream socketStringStream;//用于存储将要写入的内容
    inputstr_actul_num=recv(this_socket,inputstr,BUFLEN,0);
    //start match http
    std::smatch sm;
    std::regex regex("([A-Za-z]+) +(.*) +(HTTP/[0-9][.][0-9])");
    std::string text(inputstr);
    int match = std::regex_search(text,sm,regex);
    if(match==0){
        printf("cannot match this http\n");
         ;
    }
    // GET /index.html HTTP/1.1
    else{
        // for(unsigned i =0 ; i< sm.size();i++){
        //     //
        // }
    }
    string operation1 = sm[1];// GET
    string operation2 = sm[2];// /index.html
    std::cout <<"operation:\n"<< operation1 <<std::endl<< operation2 << std::endl;

    std::smatch sm2;
    std::regex regex2("\\..*");//匹配一个.后面接任意数量的字符
    int match2 = std::regex_search(operation2, sm2, regex2);
    //std::cout << match;
    if (match2 == 0)
    {
        printf("no file extension found, read again");
         ;
    }
    else {
        //std::cout<<sm2[0]<<std::endl;
    }
    string contentType;
    if (sm2[0] == ".html")
    {
        contentType = "text/html";
    }
    else if (sm2[0] == ".xml")
    {
        contentType = "text/xml";
    }
    else if (sm2[0] == ".css")
    {
        contentType = "text/css";
    }
    else if (sm2[0] == ".png") {
        contentType = "image/png";
    }
    else if (sm2[0] == ".gif") {
        contentType = "image/gif";
    }
    else if (sm2[0] == ".jpg") {
        contentType = "image/jpg";
    }
    else if (sm2[0] == ".jpeg") {
        contentType = "image/jpeg";
    }
    else if (sm2[0] == ".ico") {//TODO this is wrong!
        contentType = "image/ico";
    }
    else {
        contentType = "text/plain";
    }
    // 通过正则表达式以二进制打开一个文件
    std::ifstream t(fileBase+operation2, std::ios::binary);
    if (!t)
    {
        // 如果打开失败则返回404页面
        socketStringStream << ERROR_HTML_PAGE;
        string tmp = socketStringStream.str();
        const char* tmp_buf= tmp.c_str();
        int size =strlen(tmp_buf);
        write(this_socket,tmp_buf, size);
        socketStringStream.str(std::string());
         ;
    }
    //成功找到文件 开始输出
    std::filebuf* tmp= t.rdbuf();
    int size= tmp->pubseekoff(0, t.end, t.in);
    tmp->pubseekpos(0, t.in);
    // allocate memory to contain file data
    char* buffer = new char[size];
    // get file data
    tmp->sgetn(buffer, size);
    t.close();
    socketStringStream <<"HTTP/1.1 200 OK" <<"\r\n"
        << "Server: Miao\r\n"<< "Content-Length: " << size <<"\r\n"
        <<"Connection: close\r\n"<<"Content-Type: " << contentType << "\r\n\r\n";
    string tmp2 = socketStringStream.str();
    const char* tmp_buf= tmp2.c_str();
    int tmp_size =strlen(tmp_buf);
    write(this_socket,tmp_buf, tmp_size);
    socketStringStream.str(std::string());
    comming.write(buffer, size);

}

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
    srvSock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    
    if(srvSock == INVALID_SOCKET)
    {
    printf("Server create socket error!\n");
    WSACleanup();
    return 2;
    }
    printf("2.Server TCP socket create OK!\n");

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
    {[0]
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
    std::thread thread_pool[nNumConns];//线程池
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
            std::thread thread_pool[nNumConns]=std::thread(http_this,sConns[nNumConns],ConnAddrs[nNumConns]);
            thread_pool[nNumConns].detach();
            printf("5.accept and create new socket OK!\n");
            nNumConns ++;
        }
    }


    return 0;
}





