#include "Server.h"
#include <iostream>

int main() {
    try {
        Server server(8080);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Server crashed: " << e.what() << std::endl;
    }
    return 0;
}