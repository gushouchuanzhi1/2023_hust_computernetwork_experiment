#include "stdafx.h"
#include "Global.h"
#include "GBNRdtSender.h"


GBNRdtSender::GBNRdtSender() :nextSeqNum(0), waitingState(false), base(0)
{
}


GBNRdtSender::~GBNRdtSender()
{
}


bool GBNRdtSender::getWaitingState() {
	return waitingState;
}


bool GBNRdtSender::send(const Message& message) {//向网络层发送数据
	if (!waitingState) {
		this->packetWaitingAck[nextSeqNum].acknum = -1; //忽略该字段
		this->packetWaitingAck[nextSeqNum].seqnum = this->nextSeqNum;
		this->packetWaitingAck[nextSeqNum].checksum = 0;
		memcpy(this->packetWaitingAck[nextSeqNum].payload, message.data, sizeof(message.data));
		this->packetWaitingAck[nextSeqNum].checksum = pUtils->calculateCheckSum(this->packetWaitingAck[nextSeqNum]);
		pUtils->printPacket("发送方发送报文", this->packetWaitingAck[nextSeqNum]);
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[nextSeqNum]);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
		if (base == nextSeqNum) {
			pns->startTimer(SENDER, Configuration::TIME_OUT, packetWaitingAck[nextSeqNum].seqnum);			//启动发送方定时器
		}
		nextSeqNum = (nextSeqNum + 1) % GBN_seqnum;  //8 = 2^k;
		printf("***************发送报文后，当前窗口：[%d,%d]\t且base=%d，nextseqnum=%d ***************\n", base,(base-1+GBN_Winsize)%GBN_seqnum, base, nextSeqNum);
		if ((nextSeqNum-base+ GBN_seqnum)% GBN_seqnum == GBN_Winsize) {
			waitingState = true;
		}
		//printf("waitingstate = %d\n", waitingState == false ? 0 : 1);
		return true;
	}
	else {
		return false;
	}
}

void GBNRdtSender::receive(const Packet& ackPkt) {
	//如果发送方处于等待ack的状态，作如下处理；否则什么都不做
		//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	//如果校验和正确，并且确认序号在[base,base+N)区间内
	//因为接收方只接受连续序号的报文
	if (checkSum == ackPkt.checksum&& ((ackPkt.acknum-base>=0&& ackPkt.acknum - base <= GBN_Winsize)||(ackPkt.acknum+1+ GBN_seqnum -base<=GBN_Winsize))) {
		waitingState = false;
		pUtils->printPacket("发送方正确收到确认", ackPkt);
		//base++;
		int buf = base;
		base = (ackPkt.acknum + 1)% GBN_seqnum;  //base更改
		printf("***************当前窗口：[%d,%d]\t且base=%d，nextseqnum=%d ***************\n", base, (base - 1 + GBN_Winsize) % GBN_seqnum, base, nextSeqNum);
		if (base == nextSeqNum) {   //此时窗口为0，关闭该次接受的序列号对应的计时器
			pns->stopTimer(SENDER, buf);		//关闭定时器，这里seqnum是上一次starttimer的参数
		}
		else {  //重新启动计时器，参数为下一个期望接受的序列号
			pns->stopTimer(SENDER, buf);									//首先关闭定时器
			pns->startTimer(SENDER, Configuration::TIME_OUT, base);			//重新启动发送方定时器
			
		}
	}
	//else:发送方没有正确收到确认报文，忽略 ,不做处理
}

void GBNRdtSender::timeoutHandler(int seqNum) {
	printf("----------------发送方定时器到，重发窗口中序号在[%d,%d)范围内的报文,即base=%d,seqnum=%d-----------------\n", base, nextSeqNum,base,nextSeqNum);
	pns->stopTimer(SENDER, seqNum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
	int len = (this->nextSeqNum + GBN_seqnum - this->base) % GBN_seqnum;   //需重新发送的报文数目
	//重发小于nextseqnum之前的所有报文
	//for (int j = base; j < nextSeqNum; j++) {
	for (int i = 0,j = base; i<len ; i++,j=(j+1)% GBN_seqnum) {
		pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", this->packetWaitingAck[j]);
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[j]);			//重新发送数据包
	}
}
