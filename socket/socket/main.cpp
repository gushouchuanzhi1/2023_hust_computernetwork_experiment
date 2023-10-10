#pragma once//预处理指令，头文件仅被编译一次 
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "winsock2.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <ctime>   
#include <sstream> 
using namespace std;
using namespace std::filesystem;//修改c++为17版本
namespace fs = std::filesystem;
#pragma comment(lib,"ws2_32.lib")
//接受http的请求报文，从RecvBuf的缓冲池把报文里面的请求的文件
//找到GET后面的请求文件名，以二进制方式打开文件复制到响应报文里
//浏览器打开请求的报文文件
int flag = 1;
const int resp_code[5] = { 404, 200 };
const char content_type[5][20] = { "text/html", "image/jpeg", "application/pdf", "video/mp4" };
string resp_head = "HTTP/1.1 %d\r\nDate:%s\r\nContent-type:%s\r\nContent-Length:%d\r\n\r\n";
//得到GET和文件URL
int requestHandle(char* buf, char* way, char* url)
{
    if (buf == nullptr || way == nullptr || url == nullptr)
    {
        return -1;
    }
    int i = 0;
    char* ptr = nullptr;
    for (ptr = buf; *ptr != ' '; ptr++)
    {
        way[i++] = *ptr;
    }
    way[i] = '\0';
    // 跳过空格
    ptr++;
    i = 0;
    for (; *ptr != ' '; ptr++)
    {
        url[i++] = *ptr;
    }
    url[i] = '\0';
    return 0;
}
//通过url找后缀
const char* getFileType(string url)
{
    int pos = url.find_last_of(".");
    if (pos != url.npos)
    {
        // 获取文件后缀名
        string tmp = url.substr(pos);
        if (tmp == ".html")
        {
            return content_type[0];
        }
        else if (tmp == ".jpg" || tmp == ".jpeg")
        {
            return content_type[1];
        }
        else if (tmp == ".pdf")
        {
            return content_type[2];
        }
        else if (tmp == ".mp4")
        {
            return content_type[3];
        }
        else
        {
            return "";
        }
    }
}
//返回文件的大小
int getFileSize(FILE* fp)
{
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    rewind(fp);
    return size;
}
//得到现在的时间
const char* getCurrentTime()
{
    char buf[128] = { 0 };
    time_t nowtime;
    time(&nowtime); //获取1970年1月1日0点0分0秒到现在经过的秒数
    tm p;
    localtime_s(&p, &nowtime); //将秒数转换为本地时间,年从1900算起,需要+1900,月为0-11,所以要+1
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d\0", p.tm_year + 1900, p.tm_mon + 1, p.tm_mday, p.tm_hour, p.tm_min, p.tm_sec);
    return buf;
}
// 发送响应报文
int sendResponse(SOCKET fd, FILE* fp, string url)
{
    struct _stat32 stat;
    char buf[256] = { 0 };
    int type = 1;
    if (url.find("404.html") != url.npos)
    {
        type = 0;
    }
    sprintf(buf, resp_head.c_str(), resp_code[type], getCurrentTime(), getFileType(url), getFileSize(fp));
    send(fd, buf, strlen(buf), 0);
    return 0;
}
//发送文件的响应报文
int sendFile(SOCKET fd, string filename)
{
    FILE* fp = nullptr;
    if ((fp = fopen(filename.c_str(), "rb")) == nullptr)
    {
        perror("Casued By:");
        return 1;
    }
    // 首先发送响应报文
    sendResponse(fd, fp, filename);
    // 再读取并发送文件
    char* buf = new char[4096];
    memset(buf, '\0', 4096);

    while (!feof(fp))
    {
        int len = fread(buf, sizeof(char), 4096, fp);
        send(fd, buf, len, 0);
    }

    delete[] buf;
    fclose(fp);
    return 0;
}
//发送文件判断是否404
int setFiletoMessage(SOCKET fd, string url)
{
    string tmp = "G:/desktop/cpnet" + url;
    struct _stat32 stat;
    if (_stat32(tmp.c_str(), &stat) == -1)
    {
        // 不存在此文件，返回404页面
        string notfound = "G:/desktop/cpnet/404.html";
        flag = 0;
        return sendFile(fd, notfound);
    }
    else
    {
        // 存在则返回相应文件
        return sendFile(fd, tmp);
    }
}

int main() {
    unsigned long int my_addr;//set ip
    unsigned short int my_htons;//set port
    string maincontent;

	WSADATA wsaData;
	fd_set rfds;//用于检查socket是否有数据到来的的文件描述符，用于socket非阻塞模式下等待网络事件通知（有数据到来）
	fd_set wfds;
	bool first_connetion = true;//是否为用户的第一个请求
	int nRc = WSAStartup(0x0202, &wsaData);
	if (nRc) {
		printf("Winsock  startup failed with error!\n");
	}
	if (wsaData.wVersion != 0x0202) {
		printf("Winsock version is not correct!\n");
	}
	printf("Winsock startup Ok!\n");

	//创建监听socket
	SOCKET srvSocket;
	//服务器地址和客户端地址
	sockaddr_in addr, clientAddr;
	//会话socket，负责和client进程通信
    SOCKET sessionSocket = INVALID_SOCKET;//负责进程的套接字 
	int addrLen = 0;	//ip地址长度


	srvSocket = socket(AF_INET, SOCK_STREAM, 0);//第二个为流数据
	if (srvSocket != INVALID_SOCKET)
		printf("Socket create Ok!\n");

	//set port and ip
	addr.sin_family = AF_INET;
    std::ifstream inputFile("G:\\desktop\\cpnet\\ipport.txt");
    if (!inputFile.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return 1;
    }
    // read ip
    if (std::getline(inputFile, maincontent)) {
        my_addr = inet_addr(maincontent.c_str());
    }
    // read port
    if (std::getline(inputFile, maincontent)) {
        my_htons = static_cast<unsigned short int>(std::stoi(maincontent));
    }
    // read maincontent
    if (std::getline(inputFile, maincontent)) {
        cout << "读取完毕" << endl;
    }

    // print ip port address
    std::cout << "IP地址: " << my_addr << std::endl;
    std::cout << "端口号: " << my_htons << std::endl;
    std::cout << "主目录文件路径: " << maincontent << std::endl;
    inputFile.close();

    my_addr = INADDR_ANY;
    // set port ip
    addr.sin_family = AF_INET;
    addr.sin_port = htons(my_htons);
    addr.sin_addr.S_un.S_addr = htonl(my_htons);
    cout << "设置监听IP以及端口成功";

    // 设置主目录
    string wty_menu = maincontent; 
    path str(wty_menu);//利用path更方便处理
    cout << endl << "设置路径成功" << endl;

    //binding
    int rtn = bind(srvSocket, (LPSOCKADDR)&addr, sizeof(addr));
    if (rtn != SOCKET_ERROR)
        printf("Socket bind Ok!\n");

    //listen
    rtn = listen(srvSocket, 5);
    if (rtn != SOCKET_ERROR)
        printf("Socket listen Ok!\n");
    clientAddr.sin_family = AF_INET;
    addrLen = sizeof(clientAddr);

    //设置缓冲区
    char recvBuf[4096];
    u_long blockMode = 0;//将srvSock设为非阻塞模式以监听客户连接请求
    if ((rtn = ioctlsocket(srvSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO：允许或禁止套接口s的非阻塞模式。
        cout << "ioctlsocket() failed with error!\n";
        return -1;
    }
    cout << "ioctlsocket() for server socket ok!	Waiting for client connection and data\n";

    //清空read,write描述符，对rfds和wfds进行了初始化，必须用FD_ZERO先清空，下面才能FD_SET
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    //设置等待客户连接请求
    FD_SET(srvSocket, &rfds);
    
    while (true) {//start
        //清空read,write描述符，对rfds和wfds进行了初始化，必须用FD_ZERO先清空，下面才能FD_SET
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        //设置等待客户连接请求
        FD_SET(srvSocket, &rfds);
        if (!first_connetion) {
            //设置等待会话SOKCET可接受数据或可发送数据
            FD_SET(sessionSocket, &rfds);
            FD_SET(sessionSocket, &wfds);
        }
        //返回总共可以读或写的句柄个数
        int nTotal = select(0, &rfds, &wfds, NULL, NULL);
        //如果srvSock收到连接请求，接受客户连接请求
        if (FD_ISSET(srvSocket, &rfds)) {
            nTotal--;
            sessionSocket = accept(srvSocket, (LPSOCKADDR)&clientAddr, &addrLen);
            if (sessionSocket != INVALID_SOCKET) {
                printf("Socket listen one client request!\n");
                cout << "Client's IP: " << inet_ntoa(((sockaddr_in*)&clientAddr)->sin_addr) << endl;
                cout << "Client's Port: " << clientAddr.sin_port << endl;
            }
            //把会话SOCKET设为非阻塞模式
            if ((rtn = ioctlsocket(sessionSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO：允许或禁止套接口s的非阻塞模式。
                cout << "ioctlsocket() failed with error!\n";
                return 1;
            }
            cout << "ioctlsocket() for session socket ok!	Waiting for client connection and data\n";
            //设置等待会话SOKCET可接受数据或可发送数据
            FD_SET(sessionSocket, &rfds);
            FD_SET(sessionSocket, &wfds);
            first_connetion = false;
        }



        //检查会话SOCKET是否有数据到来
        if (nTotal > 0) {
            //如果会话SOCKET有数据到来，则接受客户的数据
            if (FD_ISSET(sessionSocket, &rfds)) {
                //receiving data from client
                memset(recvBuf, '\0', 4096);
                rtn = recv(sessionSocket, recvBuf, 256, 0);
                if (rtn > 0) {//表示缓冲区里读取到了htpp的请求报文
                    printf("Received %d bytes from client: %s\n", rtn, recvBuf);
                    char way[20], url[1001];
                    if (requestHandle(recvBuf, way, url) >= 0) {
                        cout << "Request URL: " << inet_ntoa(((sockaddr_in*)&clientAddr)->sin_addr) << url << endl;
                        if (strcmp("GET", way) == 0) {
                            if (setFiletoMessage(sessionSocket, url) == SOCKET_ERROR) {
                                cout << "Send Failed!!" << endl;
                                return 1;
                            }
                            else {
                                if(flag == 1) cout << "Send Success!!\n" << endl;
                                if (flag == 0) {
                                    cout << "404 NOT FOUND!\n" << endl;
                                }
                            }
                        }
                    }
                }
                else if (rtn == SOCKET_ERROR) {
                    cout << "Occur Error When Receive Data From Client: "<< endl;
                }
                else { //否则是收到了客户端断开连接的请求，也算可读事件。但是这种情况下FD_ISSET(sessionSocket, &rfds)返回false
                    printf("Client leaving ...\n");
                    closesocket(sessionSocket);  //既然client离开了，就关闭sessionSocket
                    nTotal--;	//因为客户端离开也属于可读事件，所以需要-1
                    sessionSocket = INVALID_SOCKET; //把sessionSocket设为INVALID_SOCKET
                }
            }
        }
    }
	return 0;
}



