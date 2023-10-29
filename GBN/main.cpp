// main.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"

int main(int argc, char* argv[])
{
	RdtSender* ps = new GBNRdtSender();
	RdtReceiver* pr = new GBNRdtReceiver();
	printf("采用GBN协议来进行实验------窗口长度=%d------报文序号从0-%d\n\n", GBN_Winsize, GBN_seqnum - 1);
	// pns->setRunMode(0);  //VERBOS模式
	pns->setRunMode(1);  //安静模式
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("config\\input.txt");
	pns->setOutputFile("config\\output.txt");
	pns->start();
	delete ps;
	delete pr;
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete
	return 0;
}

