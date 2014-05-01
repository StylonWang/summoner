#ifndef __INETINTERFACE_H__
#define __INETINTERFACE_H__

#include <netinet/in.h> // for INET_ADDRSTRLEN
#include <arpa/inet.h> // for inet_addr
#include <string.h> // for strncmp
#include <vector>

struct InetInterface
{
    char name[16];
    char ip[INET_ADDRSTRLEN];

//    struct sockaddr_in group_sock;
//    int sd; // socket to UDP multicast

    // constructor
    InetInterface()
    {
        name[0] = 0;
        strncpy(ip, "0.0.0.0", sizeof(ip));
    }
    InetInterface(const char *n, const char *ipaddr)
    {
        strncpy(name, n, sizeof(name));
        strncpy(ip, ipaddr, sizeof(ip));
    }

    // return all local interfaces
    static std::vector<InetInterface> getAllInterfaces(void);

};

static inline bool operator == (const InetInterface &lhs, const InetInterface &rhs)
{
    in_addr_t lhsa, rhsa;

    lhsa = inet_addr(lhs.ip);
    rhsa = inet_addr(rhs.ip);

    if(strncmp(lhs.name, rhs.name, strlen(lhs.name))==0 &&
       lhsa==rhsa) {
        return true;
    }
    else {
        return false;
    }
}

#endif //__INETINTERFACE_H__
