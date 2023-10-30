// main.cpp : �������̨Ӧ�ó������ڵ㡣
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
	printf("����GBNЭ��������ʵ��------���ڳ���=%d------������Ŵ�0-%d\n\n", GBN_Winsize, GBN_seqnum - 1);

#elif _STOPWAIT_
	RdtSender* ps = new StopWaitRdtSender();
	RdtReceiver* pr = new StopWaitRdtReceiver();

#elif _SR_
	RdtSender* ps = new SRRdtSender();
	RdtReceiver* pr = new SRRdtReceiver();
	printf("����SRЭ��������ʵ��------���ڳ���=%d------������Ŵ�0->%d\n\n", SR_Winsize, SR_seqnum - 1);

#elif _TCP_
	RdtSender* ps = new TCPRdtSender();
	RdtReceiver* pr = new TCPRdtReceiver();
	printf("����TCPЭ��������ʵ��------���ڳ���=%d------������Ŵ�0->%d\n\n", TCP_Winsize, TCP_seqnum - 1);

#else
	printf("����");
	return 0;
#endif

	// pns->setRunMode(0);  //VERBOSģʽ
	pns->setRunMode(1);  //����ģʽ
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("G:\\desktop\\input.txt");
	pns->setOutputFile("G:\\desktop\\output.txt");
	pns->start();
	delete ps;
	delete pr;
	delete pUtils;									//ָ��Ψһ�Ĺ�����ʵ����ֻ��main��������ǰdelete
	delete pns;										//ָ��Ψһ��ģ�����绷����ʵ����ֻ��main��������ǰdelete
	return 0;
}
