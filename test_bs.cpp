#include <iostream>
#include <string>
#include "ukengine_wrapper.h"

int main() {
    UkEngineWrapper ukengine;
    ukengine.init();
    int backs = 0;
    std::cout << "d: " << ukengine.processKey('d', backs) << " (backs=" << backs << ")\n";
    std::cout << "BS: " << ukengine.processKey('\b', backs) << " (backs=" << backs << ")\n";
    return 0;
}
