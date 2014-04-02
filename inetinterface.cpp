
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <vector>

#include "inetinterface.h"

std::vector<InetInterface> 
InetInterface::getAllInterfaces(void)
{
    struct ifaddrs *ifap = NULL, *ifap_orig = NULL;
    int ret;
    std::vector<InetInterface> result;

    ret = getifaddrs(&ifap);
    if(ret) return result;
    ifap_orig = ifap;

    while(ifap) {

        struct sockaddr_in *addr = (struct sockaddr_in *)ifap->ifa_addr;
        
        // skip other than IPV4
        if(AF_INET==addr->sin_family && strncmp(ifap->ifa_name, "lo", 2)) {

#if 0
            printf("%s:%d %s, %s\n", 
                    (addr->sin_family==AF_INET)? "INET" :
                    (addr->sin_family==AF_INET6)? "INET6" :
                    "OTH",
                    addr->sin_family,
                    ifap->ifa_name, 
                    inet_ntoa(addr->sin_addr)
                    );
#endif
            result.push_back(InetInterface(ifap->ifa_name, inet_ntoa(addr->sin_addr)));
        }

        ifap = ifap->ifa_next; //next interface
    }

    if(ifap_orig) freeifaddrs(ifap_orig);
    
    return result;
}

