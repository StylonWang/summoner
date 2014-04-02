
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

#include "udpmcastsender.h"
#include "inetinterface.h"
#include "sdebug.h"

int UDPMCastSender::Init(const InetInterface &iface, const char *mcastip, const short port, bool nonblocking)
{
        m_if = iface;

        memset((char *)&m_group_sock, 0, sizeof(m_group_sock)); 
        m_group_sock.sin_family = AF_INET; 
        m_group_sock.sin_addr.s_addr = inet_addr(mcastip);
        m_group_sock.sin_port = htons(port);

        m_sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(m_sd<0) return -1;

        //TODO: will we need to do this on each send? (is this affecting this socket ony?)
        // assign this interface to send multicast packets
        struct in_addr local_if;
        local_if.s_addr = inet_addr(m_if.ip);
        if (0!=setsockopt(m_sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&local_if, sizeof(local_if))) {
            ERR("cannot set local interface: %s\n", strerror(errno));
            return -1;
        }

        int loop = 1;
        if (0!=setsockopt(m_sd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop))) {
            ERR("cannot disable multicast loop: %s\n", strerror(errno));
            return -1;
        }

        loop = 3;
        socklen_t size = -5;
        getsockopt(m_sd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, &size);
        DBG("multicast looping is %d, sz%d\n", loop, size);

        // set non-blocking mode
        if(nonblocking) {
            int opt = 1;
            if(-1==ioctl(m_sd, FIONBIO, &opt)) {
                ERR("cannot set to nonblock: %s\n", strerror(errno));
                return -1;
            }
        }

        return 0; // success
}

UDPMCastSender::UDPMCastSender()
{
    m_sd = -1;
}

UDPMCastSender::~UDPMCastSender()
{
    if(m_sd!=-1) {
        m_sd = -1;
        close(m_sd);
    }
}

ssize_t 
UDPMCastSender::Send(const void *buffer, size_t length)
{
    ssize_t ret;

    ret = sendto(m_sd, buffer, length, 0, (struct sockaddr *)&m_group_sock,
                                  sizeof(m_group_sock));

    return ret;
}

