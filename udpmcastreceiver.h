#ifndef __UDP_MCAST_RECEIVER_H__
#define __UDP_MCAST_RECEIVER_H__

#include <arpa/inet.h>

#include "inetinterface.h"

class UDPMCastReceiver
{
    struct sockaddr_in m_group_sock;
    struct ip_mreq m_groupreq;
    int m_sd; // socket to UDP multicast
    
    InetInterface m_if;

    public:
        UDPMCastReceiver();
        ~UDPMCastReceiver();

        int Init(const InetInterface &iface, const char *mcastip, const short port);
        //TODO: add InitMore() to accept message from other interfaces as well

        // return number of bytes received, -1 if error, zero if no message
        int Receive(char *buf, int buflen, int timeout_in_msec=0);
};

#endif //__UDP_MCAST_RECEIVER_H__
