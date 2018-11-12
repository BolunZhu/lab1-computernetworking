#include <stdio.h>
#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <streambuf>
#include <list>
#include <algorithm>
#include <string.h>
#include <direct.h>
//#include <winsock.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
//对于Winsock 2, include <winsock2.h>
#include <thread>
#include <regex>
#include <mutex>
#define MAXCONN 10//for single thread
#define BUFLEN 255
int g_num = 0;  // protected by g_num_mutex
std::mutex g_num_mutex;
using std::string;
volatile int running = 0;
static const char ERROR_HTML_PAGE[] = "HTTP/1.1 404 Not Found\r\nContent-Type: \
text/html\r\nContent-Length: 102\r\n\r\n<HTML>\r\n<BODY>\r\n404 not found !Sorry, the page you requested was not found. By ZBL ACM1601 U201614788\
\r\n</BODY>\r\n</HTML>\r\n\0";
//必须使用\\来表示转义字符 否则\无法被识别
string fileBase = "G:\\ComputerNetworkLab\\lab1\\level1\\webserver\\lab1-computernetworking\\";

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
		written = send(s, bufp, left, 0);
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
        printf("cannot match this http , return 404!\n");
		socketStringStream << ERROR_HTML_PAGE;
		string tmp = socketStringStream.str();
		const char* tmp_buf = tmp.c_str();
		int size = strlen(tmp_buf);
		write(this_socket, tmp_buf, size);
		socketStringStream.str(std::string());
		return;
    }
    // GET /index.html HTTP/1.1
    string operation1 = sm[1];// GET
    string operation2 = sm[2];// /index.html
    
    std::smatch sm2;
    std::regex regex2("\\..*");//匹配一个.后面接任意数量的字符
    int match2 = std::regex_search(operation2, sm2, regex2);
    //std::cout << match;
    if (match2 == 0)
    {
        g_num_mutex.lock();
        std::cout <<"operation:\n"<< operation1 <<std::endl<< operation2 << std::endl;
        printf("no file extension found, return 404!\n");
        g_num_mutex.unlock();
		socketStringStream << ERROR_HTML_PAGE;
		string tmp = socketStringStream.str();
		const char* tmp_buf = tmp.c_str();
		int size = strlen(tmp_buf);
		write(this_socket, tmp_buf, size);
		socketStringStream.str(std::string());
         return;
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
        g_num_mutex.lock();
        std::cout <<"operation:\n"<< operation1 <<std::endl<< operation2 << std::endl;
        printf("Cannot open this file , return 404!\n");
        g_num_mutex.unlock();
        socketStringStream << ERROR_HTML_PAGE;
        string tmp = socketStringStream.str();
        const char* tmp_buf= tmp.c_str();
        int size =strlen(tmp_buf);
        write(this_socket,tmp_buf, size);
        socketStringStream.str(std::string());
        return;
    }
    //成功找到文件 开始输出
    std::filebuf* tmp= t.rdbuf();
    int size= tmp->pubseekoff(0, t.end, t.in);
    tmp->pubseekpos(0, t.in);
    // allocate memory to contain file data
	if (size <= 0) {
        g_num_mutex.lock();
        std::cout <<"operation:\n"<< operation1 <<std::endl<< operation2 << std::endl;
        printf("size <= 0 ! Exit!\n");
        g_num_mutex.unlock();
		socketStringStream << ERROR_HTML_PAGE;
		string tmp = socketStringStream.str();
		const char* tmp_buf = tmp.c_str();
		int size = strlen(tmp_buf);
		write(this_socket, tmp_buf, size);
		socketStringStream.str(std::string());
		return;
	}
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
    write(this_socket,buffer, size);
	return;
}

void info_input(char *t)
{
	char c;
	std::cin.get(c);
	if (c != '\n')
	{
		std::cin.putback(c);
		std::cin >> t;
		std::cin.ignore(1, '\n');   //清除输入后留下的回车，或者直接清空缓存区也可
	}
	
}
void info_input(string &t)
{
	char c;
	std::cin.get(c);
	if (c != '\n')
	{
		std::cin.putback(c);
		std::cin >> t;
		std::cin.ignore(1, '\n');   //清除输入后留下的回车，或者直接清空缓存区也可
	}

}
void info_input(unsigned int & t)
{
	char c;
	std::cin.get(c);
	if (c != '\n')
	{
		std::cin.putback(c);
		std::cin >> t;
		std::cin.ignore(1, '\n');    //清除输入后留下的回车，或者直接清空缓存区也可
	}
}
void exit_fun() {
	while (running) {
		char a = getchar();
		if (a == 'q'||a=='Q') {
			running = 0;
			return;
		}

	}
	return;
}
int main(int argc, char const *argv[])
{
    //1
    WSADATA wsaData;
    int nRc;
    //2
    SOCKET srvSock;
    //3
    sockaddr_in srvAddr;
    //5
	running = 1;
    int nAddrLen = sizeof(sockaddr);
    //获取当前程序运行的目录
	char file_buffer[_MAX_PATH];
	if (_getcwd(file_buffer, _MAX_PATH) == NULL) {
		printf("无法获得程序当前运行目录\n");
		memset(file_buffer, 0, _MAX_PATH);
	}
	else {
		std::cout << "当前程序运行目录为" << file_buffer << "，已经设置为服务器默认工作目录\n";
		fileBase = file_buffer;
	}
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
	printf("Server winsock version error!\n ");
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
	printf("Please input file path for server .Press enter for default\n");
	info_input(fileBase);
    printf("Please input your Server IP addr(default: 127.0.0.1)\n");
	char ipaddr[16] = "127.0.0.1";
	info_input(ipaddr);
	printf("Please input your Server port(default:80)\n");
	unsigned int ip_port = 80;
	info_input(ip_port);
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(ip_port);
    srvAddr.sin_addr.S_un.S_addr = inet_addr(ipaddr);
    // srvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    nRc=bind(srvSock,(LPSOCKADDR)&srvAddr,sizeof(srvAddr));
    if(nRc == SOCKET_ERROR)
    {
    printf("Server socket bind error!\n");
    closesocket(srvSock);
    WSACleanup();
    return 3;
    }
    printf("3.Server socket bind OK!\n");

    //4.listen for connect
    nRc = listen(srvSock,MAXCONN);
    if(nRc == SOCKET_ERROR)
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
    std::thread thread_pool[MAXCONN];//线程池
	printf("input 'Q' or 'q' to exit\n");
	std::thread tmp_thread = std::thread(exit_fun);
	tmp_thread.detach();
    while( nNumConns < MAXCONN){
    //每当收到客户端连接请求，创建新的会话SOCKET，保存在/	//sConns数组中
    //客户端地址保存在ConnAddrs数组中
		
    sConns[nNumConns] = accept(srvSock,&ConnAddrs[nNumConns], &nAddrLen);
        if(sConns[nNumConns] == INVALID_SOCKET)
        {
            printf("Cannot creat new socket accepted socket When accepted\n");
            //创建会话SOCKET出错处理
        }
        else
        {
			if (running == 0) {
				return 0;
			}
            //创建会话SOCKET成功，启动新的线程与客户端会话
            thread_pool[nNumConns]=std::thread(http_this,sConns[nNumConns],ConnAddrs[nNumConns]);
            thread_pool[nNumConns].detach();
            g_num_mutex.lock();
            // printf("5.accept and create new socket OK!\n");
			printf("Thread : %d ----------\n", thread_pool[nNumConns].get_id());
			printf("该请求来自 %u.%u.%u.%u", (unsigned char)ConnAddrs[nNumConns].sa_data[2],(unsigned char) ConnAddrs[nNumConns].sa_data[3], (unsigned char)ConnAddrs[nNumConns].sa_data[4], (unsigned char)ConnAddrs[nNumConns].sa_data[5]);
			printf(": %hu\n", (unsigned short)ConnAddrs[nNumConns].sa_data[0]);
            printf("---------------\n");
            g_num_mutex.unlock();
            nNumConns ++;
        }
    }


    return 0;
}





