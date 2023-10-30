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


bool GBNRdtSender::send(const Message& message) {//运输层向网络层发送数据
	if (!waitingState) {//如果没有在等待
		this->packetWaitingAck[nextSeqNum].acknum = -1; //确认序号是不需要的
		//将要发送的数据包的序列号
		this->packetWaitingAck[nextSeqNum].seqnum = this->nextSeqNum;
		//初始话校验和
		this->packetWaitingAck[nextSeqNum].checksum = 0;
		//将数据复制到数据包中的payload（datastructure）
		memcpy(this->packetWaitingAck[nextSeqNum].payload, message.data, sizeof(message.data));
		//计算要发送的数据包中的检验和
		this->packetWaitingAck[nextSeqNum].checksum = pUtils->calculateCheckSum(this->packetWaitingAck[nextSeqNum]);
		//打印序号、确认号、校验和、和数据包中的内容
		pUtils->printPacket("发送方发送报文", this->packetWaitingAck[nextSeqNum]);
		//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[nextSeqNum]);
		//base是发送窗口的基序号，之前都是已经发送而且确认的数据包，已经没有未确认的数据包，不需要等待重传了
		if (base == nextSeqNum) {//启动发送方定时器，开始发送数据
			pns->startTimer(SENDER, Configuration::TIME_OUT, packetWaitingAck[nextSeqNum].seqnum);
		}
		//更新下一个序列号
		nextSeqNum = (nextSeqNum + 1) % GBN_seqnum;
		printf("*****发送报文后，当前滑动窗口为：[%d , %d]。且base=%d，nextseqnum=%d *****\n", base, (base - 1 + GBN_Winsize) % GBN_seqnum, base, nextSeqNum);
		if ((nextSeqNum-base+ GBN_seqnum)% GBN_seqnum == GBN_Winsize) {//如果等待发送的数据包数量大于窗口大小
			waitingState = true;
		}
		return true;
	}
	else {//如果在等待
		return false;
	}
}

void GBNRdtSender::receive(const Packet& ackPkt) {
	//如果发送方处于等待ack的状态，作如下处理；否则什么都不做
	int checkSum = pUtils->calculateCheckSum(ackPkt);//检查报文首段校验和
	//检验校验和，判断ackPkt.acknum确认序号是否在发送窗口范围内，考虑ackPkt.acknum在base~base+GBN_Winsize范围内
	if (checkSum == ackPkt.checksum && ((ackPkt.acknum-base>=0 && ackPkt.acknum - base <= GBN_Winsize)||(ackPkt.acknum+1+ GBN_seqnum -base<=GBN_Winsize))) {
		waitingState = false;//开始接受，取消等待
		pUtils->printPacket("发送方正确收到确认，窗口准备滑动", ackPkt);
		int buf = base;
		base = (ackPkt.acknum + 1)% GBN_seqnum;  //base更改
		printf("*****当前窗口：[%d,%d]\t且base=%d，nextseqnum=%d *****\n", base, (base - 1 + GBN_Winsize) % GBN_seqnum, base, nextSeqNum);
		if (base == nextSeqNum) {//此时窗口为0，关闭该次接受的序列号对应的计时器
			pns->stopTimer(SENDER, buf);//关闭定时器，这里seqnum是上一次starttimer的参数
		}
		else {  //重新启动计时器，参数为下一个期望接受的序列号
			//关闭当前数据包的定时器
			pns->stopTimer(SENDER, buf);
			//重新启动发送方定时器，进行下一轮的发送数据包
			pns->startTimer(SENDER, Configuration::TIME_OUT, base);
			
		}
	}
}

void GBNRdtSender::timeoutHandler(int seqNum) {
	cout << "发送超时，回退N步" << endl;
	printf("-----发送方定时器到，重发窗口中序号在[%d, %d)范围内的报文，base=%d，seqnum=%d-----\n", base, nextSeqNum,base,nextSeqNum);
	//关闭定时器，准备进行重传
	pns->stopTimer(SENDER, seqNum);
	//重新启动发送方定时器，开始重传
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
	//需重新发送的报文数目，下一个期待的序号-基序号
	int len = (this->nextSeqNum + GBN_seqnum - this->base) % GBN_seqnum;
	for (int i = 0, j = base; i<len ; i++, j=(j+1)% GBN_seqnum) {//重传所需的数据包
		pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", this->packetWaitingAck[j]);
		//向运输层重新发送数据包
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[j]);
	}
}
