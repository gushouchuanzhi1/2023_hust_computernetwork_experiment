// main.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "StopWaitRdtSender.h"
#include "StopWaitRdtReceiver.h"

#define _STOPWAIT_ 0



int main(int argc, char* argv[])
{

	RdtSender* ps = new SRRdtSender();
	RdtReceiver* pr = new SRRdtReceiver();
	printf("����SRЭ��������ʵ��------���ڳ���=%d------������Ŵ�0->%d\n\n", SR_Winsize, SR_seqnum - 1);


	//	pns->setRunMode(0);  //VERBOSģʽ
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



