#ifndef __UDP_SENDER_H__
#define __UDP_SENDER_H__

#include <arpa/inet.h>
#include "inetinterface.h"

class UDPSender 
{

    struct sockaddr_in m_group_sock;
    struct sockaddr_in m_source_sock;
    int m_sd; // socket to UDP multicast
    
    InetInterface m_if;

    public:
        UDPSender();
        ~UDPSender();

        int Init(const InetInterface &iface, const char *destip, const short destport, bool nonblocking,
                 const short bind_src_port=-1);

        ssize_t Send(const void *buffer, size_t length);
        ssize_t SendTo(const void *buffer, size_t length, const char *destip, const short destport);
};

#endif //__UDP_SENDER_H__
