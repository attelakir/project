// Created by attelakir

#include "TcpReassembly.h"
#include "PcapLiveDeviceList.h"


#include "Widget.h"

#ifndef UNTITLED9_TCPSNIFFER_H
#define UNTITLED9_TCPSNIFFER_H

void listInterfaces();

void tcpReassemblyConnectionStartCallback(const pcpp::ConnectionData &connectionData, void *cookie);

void tcpReassemblyMsgReadyCallback(int8_t sideIndex, const pcpp::TcpStreamData &tcpData, void *cookie);

void tcpReassemblyConnectionEndCallback(const pcpp::ConnectionData &connectionData,
                                               pcpp::TcpReassembly::ConnectionEndReason reason, void *ConMngr);

void onPacketArrives(pcpp::RawPacket *packet, pcpp::PcapLiveDevice *dev, void *stats);
#endif //UNTITLED9_TCPSNIFFER_H
