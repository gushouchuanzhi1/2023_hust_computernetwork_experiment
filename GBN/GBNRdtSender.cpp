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


bool GBNRdtSender::send(const Message& message) {//�����������㷢������
	if (!waitingState) {//���û���ڵȴ�
		this->packetWaitingAck[nextSeqNum].acknum = -1; //ȷ������ǲ���Ҫ��
		//��Ҫ���͵����ݰ������к�
		this->packetWaitingAck[nextSeqNum].seqnum = this->nextSeqNum;
		//��ʼ��У���
		this->packetWaitingAck[nextSeqNum].checksum = 0;
		//�����ݸ��Ƶ����ݰ��е�payload��datastructure��
		memcpy(this->packetWaitingAck[nextSeqNum].payload, message.data, sizeof(message.data));
		//����Ҫ���͵����ݰ��еļ����
		this->packetWaitingAck[nextSeqNum].checksum = pUtils->calculateCheckSum(this->packetWaitingAck[nextSeqNum]);
		//��ӡ��š�ȷ�Ϻš�У��͡������ݰ��е�����
		pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck[nextSeqNum]);
		//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[nextSeqNum]);
		//base�Ƿ��ʹ��ڵĻ���ţ�֮ǰ�����Ѿ����Ͷ���ȷ�ϵ����ݰ����Ѿ�û��δȷ�ϵ����ݰ�������Ҫ�ȴ��ش���
		if (base == nextSeqNum) {//�������ͷ���ʱ������ʼ��������
			pns->startTimer(SENDER, Configuration::TIME_OUT, packetWaitingAck[nextSeqNum].seqnum);
		}
		//������һ�����к�
		nextSeqNum = (nextSeqNum + 1) % GBN_seqnum;
		printf("*****���ͱ��ĺ󣬵�ǰ��������Ϊ��[%d , %d]����base=%d��nextseqnum=%d *****\n", base, (base - 1 + GBN_Winsize) % GBN_seqnum, base, nextSeqNum);
		if ((nextSeqNum-base+ GBN_seqnum)% GBN_seqnum == GBN_Winsize) {//����ȴ����͵����ݰ��������ڴ��ڴ�С
			waitingState = true;
		}
		return true;
	}
	else {//����ڵȴ�
		return false;
	}
}

void GBNRdtSender::receive(const Packet& ackPkt) {
	//������ͷ����ڵȴ�ack��״̬�������´�������ʲô������
	int checkSum = pUtils->calculateCheckSum(ackPkt);//��鱨���׶�У���
	//����У��ͣ��ж�ackPkt.acknumȷ������Ƿ��ڷ��ʹ��ڷ�Χ�ڣ�����ackPkt.acknum��base~base+GBN_Winsize��Χ��
	if (checkSum == ackPkt.checksum && ((ackPkt.acknum-base>=0 && ackPkt.acknum - base <= GBN_Winsize)||(ackPkt.acknum+1+ GBN_seqnum -base<=GBN_Winsize))) {
		waitingState = false;//��ʼ���ܣ�ȡ���ȴ�
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ�ϣ�����׼������", ackPkt);
		int buf = base;
		base = (ackPkt.acknum + 1)% GBN_seqnum;  //base����
		printf("*****��ǰ���ڣ�[%d,%d]\t��base=%d��nextseqnum=%d *****\n", base, (base - 1 + GBN_Winsize) % GBN_seqnum, base, nextSeqNum);
		if (base == nextSeqNum) {//��ʱ����Ϊ0���رոôν��ܵ����кŶ�Ӧ�ļ�ʱ��
			pns->stopTimer(SENDER, buf);//�رն�ʱ��������seqnum����һ��starttimer�Ĳ���
		}
		else {  //����������ʱ��������Ϊ��һ���������ܵ����к�
			//�رյ�ǰ���ݰ��Ķ�ʱ��
			pns->stopTimer(SENDER, buf);
			//�����������ͷ���ʱ����������һ�ֵķ������ݰ�
			pns->startTimer(SENDER, Configuration::TIME_OUT, base);
			
		}
	}
}

void GBNRdtSender::timeoutHandler(int seqNum) {
	cout << "���ͳ�ʱ������N��" << endl;
	printf("-----���ͷ���ʱ�������ط������������[%d, %d)��Χ�ڵı��ģ�base=%d��seqnum=%d-----\n", base, nextSeqNum,base,nextSeqNum);
	//�رն�ʱ����׼�������ش�
	pns->stopTimer(SENDER, seqNum);
	//�����������ͷ���ʱ������ʼ�ش�
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
	//�����·��͵ı�����Ŀ����һ���ڴ������-�����
	int len = (this->nextSeqNum + GBN_seqnum - this->base) % GBN_seqnum;
	for (int i = 0, j = base; i<len ; i++, j=(j+1)% GBN_seqnum) {//�ش���������ݰ�
		pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ϴη��͵ı���", this->packetWaitingAck[j]);
		//����������·������ݰ�
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[j]);
	}
}
