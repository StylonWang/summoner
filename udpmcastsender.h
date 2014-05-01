#ifndef __UDPMCASTSENDER_H__
#define __UDPMCASTSENDER_H__

//#include <sys/types.h>
//#include <sys/socket.h>
#include <arpa/inet.h>
//#include <netinet/in.h>

#include "inetinterface.h"

class UDPMCastSender
{
    struct sockaddr_in m_group_sock;
    struct sockaddr_in m_source_sock;
    int m_sd; // socket to UDP multicast
    
    InetInterface m_if;

    public:
        UDPMCastSender();
        ~UDPMCastSender();

        int Init(const InetInterface &iface, const char *mcastip, const short port, bool nonblocking=true,
                 const short bind_src_port=-1);

        ssize_t Send(const void *buffer, size_t length);
};

#endif //__UDPMCASTSENDER_H__
