
#include "udpmcastreceiver.h"
#include "sdebug.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

UDPMCastReceiver::UDPMCastReceiver()
{
    m_sd = -1;
}

UDPMCastReceiver::~UDPMCastReceiver()
{
    if(m_sd!=-1) {
        if( setsockopt(m_sd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&m_groupreq, sizeof(m_groupreq)) < 0) {
            ERR("Dropping multicast group error: %s\n", strerror(errno));
        }
        close(m_sd);
    }
}

int UDPMCastReceiver::Receive(char *buf, int buflen, int timeout_in_msec)
{
    // for recvfrom
    struct sockaddr_in srcaddr;
    int srcaddrlen = sizeof(struct sockaddr_in);
    int ret = 0;

    // for select
    fd_set rset;
    struct timeval timeout;

    FD_ZERO(&rset);
    FD_SET(m_sd, &rset);
    //TODO: convert large mili-seconds into seconds+usec
    timeout.tv_sec = timeout_in_msec/1000;
    timeout.tv_usec = (timeout_in_msec - (timeout.tv_sec*1000))*100;

    DBG("select timeout is %ld %ld\n", (long)timeout.tv_sec, (long)timeout.tv_usec);

    // -1 is blocking
    if(-1==timeout_in_msec) {
        ret = select(m_sd+1, &rset, NULL, NULL, NULL);
    }
    // timeout_in_msec being zero means no wait
    else {
        ret = select(m_sd+1, &rset, NULL, NULL, &timeout);
    }

    if(ret<0) {
        ERR("select failed: %s\n", strerror(errno));
        return -1;
    }
    else if(0==ret || !FD_ISSET(m_sd, &rset)) {
        return 0;
    }

    memset(&srcaddr, 0, sizeof(srcaddr));
    ret = recvfrom(m_sd, buf, buflen, 0, 
            (struct sockaddr *)&srcaddr, (socklen_t *)&srcaddrlen);
    if(ret < 0) {
        ERR("Reading datagram message error: %s\n", strerror(errno));
    }
    else {
        DBG("Msg with len=%d from %s : \"0x%x:0x%x\"\n",
               ret,
               inet_ntoa(srcaddr.sin_addr),
               buf[0], buf[1]
           );
    }
    return ret;
}

int 
UDPMCastReceiver::Init(const InetInterface &iface, const char *mcastip, const short port)
{
    DBG("Init, local ip=%s, mcast ip=%s, mcast port=%d\n", iface.ip, mcastip, port);
    m_if = iface;
    
	/* Create a datagram socket on which to receive. */
	m_sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_sd < 0) {
		ERR("Opening datagram socket error: %s\n", strerror(errno));
		return -1;
	} else
		DBG("Opening datagram socket....OK.\n");

	/* Enable SO_REUSEADDR to allow multiple instances of this */
	/* application to receive copies of the multicast datagrams. */
    int reuse = 1;
    if (setsockopt
        (m_sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse,
         sizeof(reuse)) < 0) {
        ERR("Setting SO_REUSEADDR error: %s\n", strerror(errno));
        close(m_sd);
        return -1;
    } else
        DBG("Setting SO_REUSEADDR...OK.\n");

	/* Bind to the proper port number with the IP address */
	/* specified as INADDR_ANY. */
    memset(&m_group_sock, 0, sizeof(m_group_sock));
	m_group_sock.sin_family = AF_INET;
	m_group_sock.sin_port = htons(port);
	m_group_sock.sin_addr.s_addr = INADDR_ANY;
	if (bind(m_sd, (struct sockaddr *)&m_group_sock, sizeof(m_group_sock))) {
		ERR("Binding datagram socket error: %s\n", strerror(errno));
		close(m_sd);
		return -1;
	} else
		DBG("Binding datagram socket...OK.\n");

	/* Join the multicast group on the local */
	/* interface. Note that this IP_ADD_MEMBERSHIP option must be */
	/* called for each local interface over which the multicast */
	/* datagrams are to be received. */
    memset(&m_groupreq, 0, sizeof(m_groupreq));
	m_groupreq.imr_multiaddr.s_addr = inet_addr(mcastip);
	m_groupreq.imr_interface.s_addr = inet_addr(m_if.ip);
	if (setsockopt
	    (m_sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&m_groupreq,
	     sizeof(m_groupreq)) < 0) {
		ERR("Adding multicast group error: %s\n", strerror(errno));
		close(m_sd);
		return -1;
	} else
		DBG("Adding multicast group...OK.\n");

    return 0;
}

