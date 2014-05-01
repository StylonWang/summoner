#include "udpsender.h"
#include "sdebug.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

UDPSender::UDPSender()
{
    m_sd = -1;
}

UDPSender::~UDPSender()
{

}

int UDPSender::Init(const InetInterface &iface, const char *destip, const short destport, bool nonblocking,
         const short bind_src_port)
{
    m_if = iface;

    memset((char *)&m_group_sock, 0, sizeof(m_group_sock)); 
    m_group_sock.sin_family = AF_INET; 
    m_group_sock.sin_addr.s_addr = inet_addr(destip);
    m_group_sock.sin_port = htons(destport);

    m_sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(m_sd<0) return -1;

    if(-1!=bind_src_port) {
        memset((char *)&m_source_sock, 0, sizeof(m_source_sock)); 
        m_source_sock.sin_family = AF_INET; 
        m_source_sock.sin_addr.s_addr = inet_addr(iface.ip);
        m_source_sock.sin_port = htons(bind_src_port);

        if (bind(m_sd, (struct sockaddr *)&m_source_sock, sizeof(m_source_sock))) {
            ERR("Binding datagram socket error: %s\n", strerror(errno));
            close(m_sd);
            return -1;
        } else {
            DBG("Binding datagram socket...OK.\n");
        }
    }

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

ssize_t UDPSender::Send(const void *buffer, size_t length)
{
    ssize_t ret;

    ret = sendto(m_sd, buffer, length, 0, (struct sockaddr *)&m_group_sock,
                                  sizeof(m_group_sock));

    return ret;
}

ssize_t UDPSender::SendTo(const void *buffer, size_t length, const char *destip, const short destport)
{
    struct sockaddr_in sock;

    memset((char *)&sock, 0, sizeof(sock)); 
    sock.sin_family = AF_INET; 
    sock.sin_addr.s_addr = inet_addr(destip);
    sock.sin_port = htons(destport);

    ssize_t ret;

    ret = sendto(m_sd, buffer, length, 0, (struct sockaddr *)&sock,
                                  sizeof(sock));

    return ret;
}

