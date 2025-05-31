g++ -std=c++17 PacketHandler.cpp main.cpp -o packet_sniffer -lpcap -lpthread
sudo ./packet_sniffer eth0
