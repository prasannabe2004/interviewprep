#include "PacketHandler.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: sudo " << argv[0] << " <interface>\n";
        return 1;
    }

    std::string interface_name = argv[1];

    PacketReceiver receiver(interface_name);
    receiver.start();

    return 0;
}
