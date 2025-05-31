#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#include <iostream>
#include <thread>
#include <queue>
#include <map>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <memory>
#include <cstring>
#include <pcap.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>

constexpr size_t MAX_PACKET_SIZE = 65536;

class PacketHandler {
public:
    virtual void handle(const uint8_t* data, size_t len) = 0;
    virtual ~PacketHandler() = default;
};

class TCPHandler : public PacketHandler {
public:
    void handle(const uint8_t* data, size_t len) override;
};

class UDPHandler : public PacketHandler {
public:
    void handle(const uint8_t* data, size_t len) override;
};

class ICMPHandler : public PacketHandler {
public:
    void handle(const uint8_t* data, size_t len) override;
};

class Dispatcher {
public:
    Dispatcher();
    void dispatch_packet(const uint8_t* data, size_t len);

private:
    std::map<int, std::unique_ptr<PacketHandler>> handlers;
};

class PacketReceiver {
public:
    PacketReceiver(const std::string& interface_name);
    ~PacketReceiver();

    void start();

private:
    pcap_t* handle;
    std::queue<std::vector<uint8_t>> packet_queue;
    std::mutex queue_mutex;
    std::condition_variable queue_cond;
    bool running;

    Dispatcher dispatcher;

    static void pcap_callback(u_char* user, const struct pcap_pkthdr* header, const u_char* packet);
    void receiver_loop();
    void processor_loop();
};

#endif // PACKET_HANDLER_H
