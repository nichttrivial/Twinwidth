#include <iostream>

#define LOG(msg) \
    std::cout << "c" << __FILE__ << "(" << __LINE__ << "): " << msg << std::endl;
