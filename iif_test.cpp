#include "inetinterface.h"

int main(int argc, char **argv) 
{
    std::vector<InetInterface> result;

    result = InetInterface::getAllInterfaces();

    std::vector<InetInterface>::iterator it;

    for(it=result.begin(); it!=result.end(); ++it) {
        printf("interface %s, IP %s\n", it->name, it->ip);
    }
}

