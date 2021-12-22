// Created by attelakir

#include "PcapLiveDeviceList.h"
#include "TcpReassembly.h"
#include "mysql_connection.h"
#include "mysql_driver.h"
#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "TCPSniffer.h"
#include "Widget.h"

#include <cppconn/driver.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>

#include <wx/listbox.h>
#include <wx/textdlg.h>
#include <wx/wx.h>

void listInterfaces() {
    sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();

    sql::Connection *con = driver->connect("tcp://127.0.0.1:3306", "root", "rootpass");
    sql::Statement *stmt = con->createStatement();

    const std::vector<pcpp::PcapLiveDevice *> &devList = pcpp::PcapLiveDeviceList::getInstance().getPcapLiveDevicesList();

    stmt->execute("USE TCP_DATA");
    stmt->execute("CREATE TABLE IF NOT EXISTS network_interfaces(Name TEXT, IP_address TEXT)");
    stmt->execute("DELETE FROM network_interfaces");

    sql::PreparedStatement *fill_interfaces;

    fill_interfaces = con->prepareStatement("INSERT INTO network_interfaces(Name, IP_address) VALUES (?, ?)");
    for (std::vector<pcpp::PcapLiveDevice *>::const_iterator iter = devList.begin(); iter != devList.end(); iter++) {
        fill_interfaces->setString(1, (*iter)->getName());
        fill_interfaces->setString(2, (*iter)->getIPv4Address().toString());
        fill_interfaces->execute();
    }
    delete fill_interfaces;
    delete stmt;
    delete con;
}

void tcpReassemblyConnectionStartCallback(const pcpp::ConnectionData &connectionData, void *cookie) {
    conMngr *conmngr = (conMngr *) cookie;

    conmngr->flowKey.insert(std::pair<uint32_t, size_t>(connectionData.flowKey, conmngr->conCounter));

    size_t count = 0;
    conmngr->packetsCounterSide0.insert(std::pair<size_t, size_t>(conmngr->conCounter, count));
    conmngr->packetsCounterSide1.insert(std::pair<size_t, size_t>(conmngr->conCounter, count));
    ++(conmngr->conCounter);

    std::stringstream buf;
    buf << "Source:     " << connectionData.srcIP.toString() << "       Destination:    " << connectionData.dstIP.toString();
    conmngr->listbox->Append(wxString(buf.str()));
}

std::string toUTC(double time) {
    time /= 86400000;
    long intTime = time;
    time -= intTime;
    time *= 24;
    int hours = time;
    time -= hours;
    time *= 60;
    int minutes = time;
    time -= minutes;
    time *= 60;
    int seconds = time;
    time -= seconds;
    time *= 1000;
    int mseconds = time;

    std::stringstream date;
    if (hours < 10) {
        date << "0";
    }
    date << hours << ":";
    if (minutes < 10) {
        date << "0";
    }
    date << minutes << ":";
    if (seconds < 10) {
        date << "0";
    }
    date << seconds << "." << mseconds;
    return date.str();
}

void tcpReassemblyMsgReadyCallback(int8_t sideIndex, const pcpp::TcpStreamData &tcpData, void *cookie) {

    wxLongLong msecFromEpoch;
    msecFromEpoch = wxGetLocalTimeMillis();
    double msec = msecFromEpoch.ToDouble();

    sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection *con = driver->connect("tcp://127.0.0.1:3306", "root", "rootpass");
    sql::Statement *stmt = con->createStatement();

    conMngr *conmngr = (conMngr *) cookie;

    std::string time = toUTC(msec);

    stmt->execute("USE TCP_DATA");

    std::stringstream stringstreamData;
    stringstreamData.write((char *) tcpData.getData(), tcpData.getDataLength());

    std::string data = stringstreamData.str();


    std::replace_if(
            data.begin(), data.end(), [](char item) { return (uint8_t) item < 32 || (uint8_t) item > 126; }, '.');

    sql::PreparedStatement *fill_packetData;
    sql::PreparedStatement *fill_Data;
    uint32_t flowkey = tcpData.getConnectionData().flowKey;


    if (sideIndex == 0) {
        fill_packetData = con->prepareStatement(
                "INSERT INTO PacketsSide0(ConNumber, Time, PacketNumber) VALUES (?, ?, ?)");
        fill_packetData->setInt(1, conmngr->flowKey.at(flowkey));
        fill_packetData->setString(2, (time));
        fill_packetData->setInt(3, conmngr->packetsCounterSide0.at(conmngr->flowKey.at(flowkey)));

        fill_Data = con->prepareStatement(
                "INSERT INTO DataFrom0(ConNumber, PacketNumber, Data) VALUES (?, ?, ?)");
        fill_Data->setInt(1, conmngr->flowKey.at(flowkey));
        fill_Data->setInt(2, conmngr->packetsCounterSide0.at(conmngr->flowKey.at(flowkey)));
        fill_Data->setString(3, data);

        ++(conmngr->packetsCounterSide0.at(conmngr->flowKey.at(flowkey)));
    } else {
        fill_packetData = con->prepareStatement(
                "INSERT INTO PacketsSide1(ConNumber, Time, PacketNumber) VALUES (?, ?, ?)");
        fill_packetData->setInt(1, conmngr->flowKey.at(flowkey));
        fill_packetData->setString(2, (time));
        fill_packetData->setInt(3, conmngr->packetsCounterSide1.at(conmngr->flowKey.at(flowkey)));

        fill_Data = con->prepareStatement(
                "INSERT INTO DataFrom1(ConNumber, PacketNumber, Data) VALUES (?, ?, ?)");
        fill_Data->setInt(1, conmngr->flowKey.at(flowkey));
        fill_Data->setInt(2, conmngr->packetsCounterSide1.at(conmngr->flowKey.at(flowkey)));
        fill_Data->setString(3, data);

        ++(conmngr->packetsCounterSide1.at(conmngr->flowKey.at(flowkey)));
    }
    fill_packetData->execute();
    fill_Data->execute();
    delete stmt;
    delete fill_packetData;
    delete fill_Data;
    delete con;
}

void tcpReassemblyConnectionEndCallback(const pcpp::ConnectionData &connectionData,
                                        pcpp::TcpReassembly::ConnectionEndReason reason, void *ConMngr) {
}

void onPacketArrives(pcpp::RawPacket *packet, pcpp::PcapLiveDevice *dev, void *stats) {
    pcpp::TcpReassembly *tcpReassembly = (pcpp::TcpReassembly *) stats;
    tcpReassembly->reassemblePacket(packet);
}
