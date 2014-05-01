#ifndef __UDP_MCAST_RECEIVER_H__
#define __UDP_MCAST_RECEIVER_H__

#include <arpa/inet.h>

#include "inetinterface.h"

class UDPMCastReceiver
{
    //TODO: move this to be local variable
    //struct sockaddr_in m_group_sock;

    struct MCastSocket {
        struct ip_mreq m_groupreq;
        int m_sd; // socket to UDP multicast

        InetInterface m_if;
        char m_mcastip[16];
        short m_port;
    };

    std::vector<MCastSocket> m_sockets; // sockets to UDP multicast

    int InitOne(const InetInterface &iface, const char *mcastip, const short port);

    public:
        UDPMCastReceiver();
        ~UDPMCastReceiver();

        //int Init(const InetInterface &iface, const char *mcastip, const short port);
        //Init() to accept message from other interfaces as well
        int Init(std::vector<InetInterface> &ifaces, const char *mcastip, const short port);

        // return number of bytes received, -1 if error, zero if no message
        int Receive(char *buf, int buflen, int timeout_in_msec=0);
};

#endif //__UDP_MCAST_RECEIVER_H__
