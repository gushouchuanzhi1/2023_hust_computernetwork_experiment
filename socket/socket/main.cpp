#pragma once//Ԥ����ָ�ͷ�ļ���������һ�� 
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
using namespace std::filesystem;//�޸�c++Ϊ17�汾
namespace fs = std::filesystem;
#pragma comment(lib,"ws2_32.lib")
//����http�������ģ���RecvBuf�Ļ���ذѱ��������������ļ�
//�ҵ�GET����������ļ������Զ����Ʒ�ʽ���ļ����Ƶ���Ӧ������
//�����������ı����ļ�
int flag = 1;
const int resp_code[5] = { 404, 200 };
const char content_type[5][20] = { "text/html", "image/jpeg", "application/pdf", "video/mp4" };
string resp_head = "HTTP/1.1 %d\r\nDate:%s\r\nContent-type:%s\r\nContent-Length:%d\r\n\r\n";
//�õ�GET���ļ�URL
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
    // �����ո�
    ptr++;
    i = 0;
    for (; *ptr != ' '; ptr++)
    {
        url[i++] = *ptr;
    }
    url[i] = '\0';
    return 0;
}
//ͨ��url�Һ�׺
const char* getFileType(string url)
{
    int pos = url.find_last_of(".");
    if (pos != url.npos)
    {
        // ��ȡ�ļ���׺��
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
//�����ļ��Ĵ�С
int getFileSize(FILE* fp)
{
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    rewind(fp);
    return size;
}
//�õ����ڵ�ʱ��
const char* getCurrentTime()
{
    char buf[128] = { 0 };
    time_t nowtime;
    time(&nowtime); //��ȡ1970��1��1��0��0��0�뵽���ھ���������
    tm p;
    localtime_s(&p, &nowtime); //������ת��Ϊ����ʱ��,���1900����,��Ҫ+1900,��Ϊ0-11,����Ҫ+1
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d\0", p.tm_year + 1900, p.tm_mon + 1, p.tm_mday, p.tm_hour, p.tm_min, p.tm_sec);
    return buf;
}
// ������Ӧ����
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
//�����ļ�����Ӧ����
int sendFile(SOCKET fd, string filename)
{
    FILE* fp = nullptr;
    if ((fp = fopen(filename.c_str(), "rb")) == nullptr)
    {
        perror("Casued By:");
        return 1;
    }
    // ���ȷ�����Ӧ����
    sendResponse(fd, fp, filename);
    // �ٶ�ȡ�������ļ�
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
//�����ļ��ж��Ƿ�404
int setFiletoMessage(SOCKET fd, string url)
{
    string tmp = "G:/desktop/cpnet" + url;
    struct _stat32 stat;
    if (_stat32(tmp.c_str(), &stat) == -1)
    {
        // �����ڴ��ļ�������404ҳ��
        string notfound = "G:/desktop/cpnet/404.html";
        flag = 0;
        return sendFile(fd, notfound);
    }
    else
    {
        // �����򷵻���Ӧ�ļ�
        return sendFile(fd, tmp);
    }
}

int main() {
    unsigned long int my_addr;//set ip
    unsigned short int my_htons;//set port
    string maincontent;

	WSADATA wsaData;
	fd_set rfds;//���ڼ��socket�Ƿ������ݵ����ĵ��ļ�������������socket������ģʽ�µȴ������¼�֪ͨ�������ݵ�����
	fd_set wfds;
	bool first_connetion = true;//�Ƿ�Ϊ�û��ĵ�һ������
	int nRc = WSAStartup(0x0202, &wsaData);
	if (nRc) {
		printf("Winsock  startup failed with error!\n");
	}
	if (wsaData.wVersion != 0x0202) {
		printf("Winsock version is not correct!\n");
	}
	printf("Winsock startup Ok!\n");

	//��������socket
	SOCKET srvSocket;
	//��������ַ�Ϳͻ��˵�ַ
	sockaddr_in addr, clientAddr;
	//�Ựsocket�������client����ͨ��
    SOCKET sessionSocket = INVALID_SOCKET;//������̵��׽��� 
	int addrLen = 0;	//ip��ַ����


	srvSocket = socket(AF_INET, SOCK_STREAM, 0);//�ڶ���Ϊ������
	if (srvSocket != INVALID_SOCKET)
		printf("Socket create Ok!\n");

	//set port and ip
	addr.sin_family = AF_INET;
    std::ifstream inputFile("G:\\desktop\\cpnet\\ipport.txt");
    if (!inputFile.is_open()) {
        std::cerr << "�޷����ļ�" << std::endl;
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
        cout << "��ȡ���" << endl;
    }

    // print ip port address
    std::cout << "IP��ַ: " << my_addr << std::endl;
    std::cout << "�˿ں�: " << my_htons << std::endl;
    std::cout << "��Ŀ¼�ļ�·��: " << maincontent << std::endl;
    inputFile.close();

    my_addr = INADDR_ANY;
    // set port ip
    addr.sin_family = AF_INET;
    addr.sin_port = htons(my_htons);
    addr.sin_addr.S_un.S_addr = htonl(my_htons);
    cout << "���ü���IP�Լ��˿ڳɹ�";

    // ������Ŀ¼
    string wty_menu = maincontent; 
    path str(wty_menu);//����path�����㴦��
    cout << endl << "����·���ɹ�" << endl;

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

    //���û�����
    char recvBuf[4096];
    u_long blockMode = 0;//��srvSock��Ϊ������ģʽ�Լ����ͻ���������
    if ((rtn = ioctlsocket(srvSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO��������ֹ�׽ӿ�s�ķ�����ģʽ��
        cout << "ioctlsocket() failed with error!\n";
        return -1;
    }
    cout << "ioctlsocket() for server socket ok!	Waiting for client connection and data\n";

    //���read,write����������rfds��wfds�����˳�ʼ����������FD_ZERO����գ��������FD_SET
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    //���õȴ��ͻ���������
    FD_SET(srvSocket, &rfds);
    
    while (true) {//start
        //���read,write����������rfds��wfds�����˳�ʼ����������FD_ZERO����գ��������FD_SET
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        //���õȴ��ͻ���������
        FD_SET(srvSocket, &rfds);
        if (!first_connetion) {
            //���õȴ��ỰSOKCET�ɽ������ݻ�ɷ�������
            FD_SET(sessionSocket, &rfds);
            FD_SET(sessionSocket, &wfds);
        }
        //�����ܹ����Զ���д�ľ������
        int nTotal = select(0, &rfds, &wfds, NULL, NULL);
        //���srvSock�յ��������󣬽��ܿͻ���������
        if (FD_ISSET(srvSocket, &rfds)) {
            nTotal--;
            sessionSocket = accept(srvSocket, (LPSOCKADDR)&clientAddr, &addrLen);
            if (sessionSocket != INVALID_SOCKET) {
                printf("Socket listen one client request!\n");
                cout << "Client's IP: " << inet_ntoa(((sockaddr_in*)&clientAddr)->sin_addr) << endl;
                cout << "Client's Port: " << clientAddr.sin_port << endl;
            }
            //�ѻỰSOCKET��Ϊ������ģʽ
            if ((rtn = ioctlsocket(sessionSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO��������ֹ�׽ӿ�s�ķ�����ģʽ��
                cout << "ioctlsocket() failed with error!\n";
                return 1;
            }
            cout << "ioctlsocket() for session socket ok!	Waiting for client connection and data\n";
            //���õȴ��ỰSOKCET�ɽ������ݻ�ɷ�������
            FD_SET(sessionSocket, &rfds);
            FD_SET(sessionSocket, &wfds);
            first_connetion = false;
        }



        //���ỰSOCKET�Ƿ������ݵ���
        if (nTotal > 0) {
            //����ỰSOCKET�����ݵ���������ܿͻ�������
            if (FD_ISSET(sessionSocket, &rfds)) {
                //receiving data from client
                memset(recvBuf, '\0', 4096);
                rtn = recv(sessionSocket, recvBuf, 256, 0);
                if (rtn > 0) {//��ʾ���������ȡ����htpp��������
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
                else { //�������յ��˿ͻ��˶Ͽ����ӵ�����Ҳ��ɶ��¼����������������FD_ISSET(sessionSocket, &rfds)����false
                    printf("Client leaving ...\n");
                    closesocket(sessionSocket);  //��Ȼclient�뿪�ˣ��͹ر�sessionSocket
                    nTotal--;	//��Ϊ�ͻ����뿪Ҳ���ڿɶ��¼���������Ҫ-1
                    sessionSocket = INVALID_SOCKET; //��sessionSocket��ΪINVALID_SOCKET
                }
            }
        }
    }
	return 0;
}



