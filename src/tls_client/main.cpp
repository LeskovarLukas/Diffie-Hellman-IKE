#include <iostream>
#include "include/pipe.h"

int main() {
    Pipe pipe;
    pipe << "ClientHello! ";
    std::string message;
    pipe >> message;
    std::cout << "Received: " << message << std::endl;
    return 0;
}