#include <iostream>
#include <string>
#include <unordered_map>

class RoutingTable {
  public:
    void insert(const std::string& destination, const std::string& nextHop) {
        table[destination] = nextHop;
    }

    void remove(const std::string& destination) {
        table.erase(destination);
    }

    std::string forward(const std::string& destination) {
        auto it = table.find(destination);
        if (it != table.end()) {
            return it->second;
        }
        return "No route found";
    }

  private:
    std::unordered_map<std::string, std::string> table;
};
/*
Design a routing table with insert, delete, and forward functionality. Define its class as well.
*/
