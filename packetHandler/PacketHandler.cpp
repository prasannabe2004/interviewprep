#include "PacketHandler.h"

void TCPHandler::handle(const uint8_t* data, size_t len) {
    const struct tcphdr* tcp = reinterpret_cast<const struct tcphdr*>(data);
    std::cout << "[TCP] Src Port: " << ntohs(tcp->source)
              << ", Dst Port: " << ntohs(tcp->dest) << "\n";
}

void UDPHandler::handle(const uint8_t* data, size_t len) {
    const struct udphdr* udp = reinterpret_cast<const struct udphdr*>(data);
    std::cout << "[UDP] Src Port: " << ntohs(udp->source)
              << ", Dst Port: " << ntohs(udp->dest) << "\n";
}

void ICMPHandler::handle(const uint8_t* data, size_t len) {
    const struct icmphdr* icmp = reinterpret_cast<const struct icmphdr*>(data);
    std::cout << "[ICMP] Type: " << static_cast<int>(icmp->type)
              << ", Code: " << static_cast<int>(icmp->code) << "\n";
}

Dispatcher::Dispatcher() {
    handlers[IPPROTO_TCP] = std::make_unique<TCPHandler>();
    handlers[IPPROTO_UDP] = std::make_unique<UDPHandler>();
    handlers[IPPROTO_ICMP] = std::make_unique<ICMPHandler>();
}

void Dispatcher::dispatch_packet(const uint8_t* data, size_t len) {
    if (len < sizeof(struct ip)) return;

    const struct ip* iphdr = reinterpret_cast<const struct ip*>(data);
    int ip_header_len = iphdr->ip_hl * 4;

    int protocol = iphdr->ip_p;
    if (handlers.count(protocol)) {
        handlers[protocol]->handle(data + ip_header_len, len - ip_header_len);
    }
}

PacketReceiver::PacketReceiver(const std::string& interface_name)
    : running(true) {
    char errbuf[PCAP_ERRBUF_SIZE];
    handle = pcap_open_live(interface_name.c_str(), MAX_PACKET_SIZE, 1, 1000,
                            errbuf);
    if (!handle) {
        std::cerr << "pcap_open_live() failed: " << errbuf << std::endl;
        exit(1);
    }
}

PacketReceiver::~PacketReceiver() {
    running = false;
    pcap_close(handle);
}

void PacketReceiver::start() {
    std::thread processor(&PacketReceiver::processor_loop, this);

    // Blocking loop
    pcap_loop(handle, 0, PacketReceiver::pcap_callback,
              reinterpret_cast<u_char*>(this));

    processor.join();
}

void PacketReceiver::pcap_callback(u_char* user,
                                   const struct pcap_pkthdr* header,
                                   const u_char* packet) {
    PacketReceiver* self = reinterpret_cast<PacketReceiver*>(user);

    // Skip Ethernet header (assume Ethernet for simplicity)
    const uint8_t* ip_packet = packet + 14;
    size_t ip_len = header->len - 14;

    std::vector<uint8_t> pkt(ip_packet, ip_packet + ip_len);

    {
        std::lock_guard<std::mutex> lock(self->queue_mutex);
        self->packet_queue.push(std::move(pkt));
    }
    self->queue_cond.notify_one();
}

void PacketReceiver::processor_loop() {
    while (running) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_cond.wait(lock, [this] { return !packet_queue.empty(); });

        auto packet = std::move(packet_queue.front());
        packet_queue.pop();
        lock.unlock();

        dispatcher.dispatch_packet(packet.data(), packet.size());
    }
}
