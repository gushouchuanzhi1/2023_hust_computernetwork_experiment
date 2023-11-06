// main.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "StopWaitRdtSender.h"
#include "StopWaitRdtReceiver.h"

#define _STOPWAIT_ 0
#define _GBN_ 1
#define _SR_ 0
#define _TCP_ 0


int main(int argc, char* argv[])
{
#if _GBN_
	RdtSender* ps = new GBNRdtSender();
	RdtReceiver* pr = new GBNRdtReceiver();
	printf("采用GBN协议来进行实验------窗口长度=%d------报文序号从0-%d\n\n", GBN_Winsize, GBN_seqnum - 1);

#elif _STOPWAIT_
	RdtSender* ps = new StopWaitRdtSender();
	RdtReceiver* pr = new StopWaitRdtReceiver();

#elif _SR_
	RdtSender* ps = new SRRdtSender();
	RdtReceiver* pr = new SRRdtReceiver();
	printf("采用SR协议来进行实验------窗口长度=%d------报文序号从0->%d\n\n", SR_Winsize, SR_seqnum - 1);

#elif _TCP_
	RdtSender* ps = new TCPRdtSender();
	RdtReceiver* pr = new TCPRdtReceiver();
	printf("采用TCP协议来进行实验------窗口长度=%d------报文序号从0->%d\n\n", TCP_Winsize, TCP_seqnum - 1);

#else
	printf("返回");
	return 0;
#endif

	// pns->setRunMode(0);  //VERBOS模式
	pns->setRunMode(1);  //安静模式
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("G:\\desktop\\input.txt");
	pns->setOutputFile("G:\\desktop\\output.txt");
	pns->start();
	delete ps;
	delete pr;
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete
	return 0;
}

