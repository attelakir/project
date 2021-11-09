#include <iostream>
#include "/usr/include/pcap.h"
#include "/usr/local/include/pcapplusplus/PcapLiveDeviceList.h"
#include "/usr/local/include/pcapplusplus/SystemUtils.h"

struct PacketStats {
    int ethPacketCount;
    int ipv4PacketCount;
    int ipv6PacketCount;
    int tcpPacketCount;
    int udpPacketCount;
    int dnsPacketCount;
    int httpPacketCount;
    int sslPacketCount;

    void clear() {
        ethPacketCount = 0;
        ipv4PacketCount = 0;
        ipv6PacketCount = 0;
        tcpPacketCount = 0;
        udpPacketCount = 0;
        tcpPacketCount = 0;
        dnsPacketCount = 0;
        httpPacketCount = 0;
        sslPacketCount = 0;
    }

    PacketStats() { clear(); }

    void consumePacket(pcpp::Packet &packet) {
        if (packet.isPacketOfType(pcpp::Ethernet))
            ethPacketCount++;
        if (packet.isPacketOfType(pcpp::IPv4))
            ipv4PacketCount++;
        if (packet.isPacketOfType(pcpp::IPv6))
            ipv6PacketCount++;
        if (packet.isPacketOfType(pcpp::TCP))
            tcpPacketCount++;
        if (packet.isPacketOfType(pcpp::UDP))
            udpPacketCount++;
        if (packet.isPacketOfType(pcpp::DNS))
            dnsPacketCount++;
        if (packet.isPacketOfType(pcpp::HTTP))
            httpPacketCount++;
        if (packet.isPacketOfType(pcpp::SSL))
            sslPacketCount++;
    }

    void printToConsole() {
        std::cout
                << "Ethernet packet count: " << ethPacketCount << std::endl
                << "IPv4 packet count:     " << ipv4PacketCount << std::endl
                << "IPv6 packet count:     " << ipv6PacketCount << std::endl
                << "TCP packet count:      " << tcpPacketCount << std::endl
                << "UDP packet count:      " << udpPacketCount << std::endl
                << "DNS packet count:      " << dnsPacketCount << std::endl
                << "HTTP packet count:     " << httpPacketCount << std::endl
                << "SSL packet count:      " << sslPacketCount << std::endl;
    }
};

static bool onPacketArrivesBlockingMode(pcpp::RawPacket *packet, pcpp::PcapLiveDevice *dev, void *cookie) {
    auto *stats = (PacketStats *) cookie;

    pcpp::Packet parsedPacket(packet);

    stats->consumePacket(parsedPacket);

    return false;
}

int main() {
    std::string interfaceIPAddr = "192.168.1.102";

    pcpp::PcapLiveDevice *dev = pcpp::PcapLiveDeviceList::getInstance().getPcapLiveDeviceByIp(interfaceIPAddr);
    if (dev == nullptr) {
        std::cerr << "Cannot find interface with IPv4 address of '" << interfaceIPAddr << "'" << std::endl;
        return 1;
    }

    if (!dev->open())
    {
        std::cerr << "Cannot open device" << std::endl;
        return 1;
    }

    PacketStats stats;

    std::cout << std::endl << "Starting capture in blocking mode..." << std::endl;

    stats.clear();

    dev->startCaptureBlockingMode(onPacketArrivesBlockingMode, &stats, 10);

    std::cout << "Results:" << std::endl;
    stats.printToConsole();
}