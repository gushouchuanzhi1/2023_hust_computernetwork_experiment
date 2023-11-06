#include "stdafx.h"
#include "Global.h"
#include "SRRdtReceiver.h"


SRRdtReceiver::SRRdtReceiver() :rcvbase(0)
{
	for (int i = 0; i < SR_seqnum; i++) {
		rcvseq[i] = false;
	}
	lastAckPkt.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//���Ը��ֶ�
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}


SRRdtReceiver::~SRRdtReceiver()
{
}

void SRRdtReceiver::receive(const Packet& packet) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);

	//���У�����ȷ
	if (checkSum == packet.checksum) {
		if ((packet.seqnum - rcvbase >= 0 && packet.seqnum - rcvbase <= SR_Winsize-1) || (packet.seqnum + 1 + SR_seqnum - rcvbase <= SR_Winsize)) {
			if (packet.seqnum == rcvbase) {
				Message msg;
				memcpy(msg.data, packet.payload, sizeof(packet.payload));
				int flag=rcvbase;  //[rvcbase,flag]Ϊ����Ӧ�ò�ı����������
				for (int i = (rcvbase+1)% SR_seqnum,j=1; j<SR_Winsize; j++,i=(i+1)% SR_seqnum) {
					if (rcvseq[i]==true) flag = i;
					else break;
				}
				if (flag == rcvbase) {	//û�����򵽴������
					printf("���շ�û�л���������ı��ģ�ֱ�ӽ��������Ϊ%d�ı��ĵݽ���Ӧ�ò�\n",rcvbase);
					pns->delivertoAppLayer(RECEIVER, msg);
				}
				else {	//���򵽴�ķ��͸�Ӧ�ò�
					printf("���շ�������� %d ~ %d �ı��ĵݽ���Ӧ�ò�\n", rcvbase, flag);
					pns->delivertoAppLayer(RECEIVER, msg);
					for (int i = (rcvbase + 1) % SR_seqnum, j = 0; j < (flag - rcvbase + SR_seqnum) % SR_seqnum; j++, i = (i + 1) % SR_seqnum) {
						pns->delivertoAppLayer(RECEIVER, rcvmsg[i]);
						rcvseq[i] = false;
					}
				}
				rcvbase = (flag + 1) % SR_seqnum;
				printf("*****�ݽ����ĺ󣬽��շ���ǰ���ڣ�[%d,%d]*****\n", rcvbase, (rcvbase - 1 + SR_Winsize) % SR_seqnum);
			}
			else {
				memcpy(rcvmsg[packet.seqnum].data, packet.payload, sizeof(packet.payload));
				rcvseq[packet.seqnum] = true;
				printf("������Ų����������ܷ������Ϊ%d�ı��Ļ��棬��ǰ���ڻ����rcvbase=%d\n", packet.seqnum, rcvbase);
			}

			lastAckPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
		}
		else { //���������[rcvbase-N,rcvbase-1]֮�䣬�ط�n��ȷ�ϱ���
			lastAckPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("���շ��յ���ȷ�ϱ��ģ����·���ȷ�ϱ���", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
		}
	}
	//У��ʹ�������Ӧ
}