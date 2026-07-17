#include <ibus.h>
#include <iostream>
int main() {
    IBusEngineClass c;
    std::cout << sizeof(c) << std::endl;
    return 0;
}
