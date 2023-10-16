// main.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "StopWaitRdtSender.h"
#include "StopWaitRdtReceiver.h"

#define _STOPWAIT_ 0
#define _GBN_ 0
#define _SR_ 0
#define _TCP_ 1


int main(int argc, char* argv[])
{
	FILE* outputFile = freopen("G:\\desktop\\stdoutput.txt", "w", stdout);
#if _GBN_
	RdtSender *ps = new GBNRdtSender();
	RdtReceiver * pr = new GBNRdtReceiver();
	printf("下面实现GBN协议------窗口长度=%d------报文序号从0-%d\n\n",GBN_Winsize,GBN_seqnum-1);

#elif _STOPWAIT_
	RdtSender* ps = new StopWaitRdtSender();
	RdtReceiver* pr = new StopWaitRdtReceiver();

#elif _SR_
	RdtSender* ps = new SRRdtSender();
	RdtReceiver* pr = new SRRdtReceiver();
	printf("下面实现SR协议------窗口长度=%d------报文序号从0->%d\n\n", SR_Winsize, SR_seqnum-1);

#elif _TCP_
	RdtSender* ps = new TCPRdtSender();
	RdtReceiver* pr = new TCPRdtReceiver();
	printf("下面实现TCP协议------窗口长度=%d------报文序号从0->%d\n\n", TCP_Winsize, TCP_seqnum-1);

#else
	printf("没有协议调用，返回");
	return 0;
#endif

//	pns->setRunMode(0);  //VERBOS模式
	pns->setRunMode(1);  //安静模式
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("G:\\desktop\\可靠数据传输\\config\\input.txt");
	pns->setOutputFile("G:\\desktop\\可靠数据传输\\config\\output.txt");
	pns->start();
	delete ps;
	delete pr;
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete
	fclose(outputFile);
	return 0;
}

