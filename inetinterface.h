#ifndef __INETINTERFACE_H__
#define __INETINTERFACE_H__

#include <netinet/in.h> // for INET_ADDRSTRLEN
#include <vector>

struct InetInterface
{
    char name[16];
    char ip[INET_ADDRSTRLEN];

//    struct sockaddr_in group_sock;
//    int sd; // socket to UDP multicast

    // constructor
    InetInterface(const char *n, const char *ipaddr)
    {
        strncpy(name, n, sizeof(name));
        strncpy(ip, ipaddr, sizeof(ip));
    }

    static std::vector<InetInterface> getAllInterfaces(void);
};

#endif //__INETINTERFACE_H__
