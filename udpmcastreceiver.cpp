
#include "udpmcastreceiver.h"
#include "sdebug.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

UDPMCastReceiver::UDPMCastReceiver()
{
    //m_sd = -1;
}

UDPMCastReceiver::~UDPMCastReceiver()
{
    std::vector<MCastSocket>::iterator it;

    // find redundant sockets and do nothing if found
    for(it=m_sockets.begin(); it!=m_sockets.end(); ++it) {

        if( it->m_mcastip[0]!=0 && setsockopt(it->m_sd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&(it->m_groupreq), 
                    sizeof(it->m_groupreq)) < 0) {
            ERR("Dropping multicast group error: %s\n", strerror(errno));
        }
        close(it->m_sd);
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
    int largest_sd = -1;

    FD_ZERO(&rset);

    // add all sockets to receive set
    std::vector<MCastSocket>::iterator it;
    for(it=m_sockets.begin(); it!=m_sockets.end(); ++it) {
        
        FD_SET(it->m_sd, &rset);
        if(it->m_sd > largest_sd) largest_sd = it->m_sd;
        DBG("add sd %d in set, lgsd=%d\n", it->m_sd, largest_sd);
    }

    //TODO: convert large mili-seconds into seconds+usec
    timeout.tv_sec = timeout_in_msec/1000;
    timeout.tv_usec = (timeout_in_msec - (timeout.tv_sec*1000))*100;

    DBG("select timeout is %ld %ld\n", (long)timeout.tv_sec, (long)timeout.tv_usec);

    // -1 is blocking
    if(-1==timeout_in_msec) {
        ret = select(largest_sd+1, &rset, NULL, NULL, NULL);
    }
    // timeout_in_msec being zero means no wait
    else {
        ret = select(largest_sd+1, &rset, NULL, NULL, &timeout);
    }

    if(ret<0) {
        ERR("select failed: %s\n", strerror(errno));
        return -1;
    }
    else if(0==ret) {
        return 0;
    }

    // identify the socket with data
    int sd = -1;
    for(it=m_sockets.begin(); it!=m_sockets.end(); ++it) {
        if(FD_ISSET(it->m_sd, &rset)) {
            DBG("sd %d in set\n", sd);
            sd = it->m_sd;
            break;
        }
    }

    if(-1!=sd) {

        memset(&srcaddr, 0, sizeof(srcaddr));
        ret = recvfrom(sd, buf, buflen, 0, 
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
    }
    return ret;
}

int 
UDPMCastReceiver::InitOne(const InetInterface &iface, const char *mcastip, const short port)
{
    std::vector<MCastSocket>::iterator it;

    // find redundant sockets and do nothing if found
    for(it=m_sockets.begin(); it!=m_sockets.end(); ++it) {
        
        if(NULL==mcastip) { // UDP unicast
            if(iface==it->m_if && port==it->m_port) {
                DBG("alread have socket on %s for port %d\n", iface.ip, port);
                return 0; // do nothing
            }
        }
        else { // UDP multicase
            if(iface==it->m_if && strncmp(it->m_mcastip, mcastip, sizeof(it->m_mcastip))==0 &&
                    port==it->m_port) {
                DBG("alread have socket on %s for %s %d\n", iface.ip, mcastip, port);
                return 0; // do nothing
            }
        }
    }

    MCastSocket s;

    s.m_if = iface;
    if(NULL==mcastip) {
        s.m_mcastip[0] = 0; // empty string, to signify this is a unicast receiver
    }
    else {
        strncpy(s.m_mcastip, mcastip, sizeof(s.m_mcastip));
    }
    s.m_port = port;

    DBG("Init, local ip=%s, mcast ip=%s, mcast port=%d\n", iface.ip, mcastip, port);
    
	/* Create a datagram socket on which to receive. */
	s.m_sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (s.m_sd < 0) {
		ERR("Opening datagram socket error: %s\n", strerror(errno));
		return -1;
	} else
		DBG("Opening datagram socket....OK.\n");

	/* Enable SO_REUSEADDR to allow multiple instances of this */
	/* application to receive copies of the multicast datagrams. */
    int reuse = 1;
    if (setsockopt
        (s.m_sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse,
         sizeof(reuse)) < 0) {
        ERR("Setting SO_REUSEADDR error: %s\n", strerror(errno));
        close(s.m_sd);
        return -1;
    } else
        DBG("Setting SO_REUSEADDR...OK.\n");

	/* Bind to the proper port number with the IP address */
	/* specified as INADDR_ANY. */
    struct sockaddr_in group_sock;
    memset(&group_sock, 0, sizeof(group_sock));
	group_sock.sin_family = AF_INET;
	group_sock.sin_port = htons(port);
	group_sock.sin_addr.s_addr = INADDR_ANY; // must be INADDR_ANY to receive multicast
	//group_sock.sin_addr.s_addr = inet_addr(s.m_if.ip);

    // it is normal if bind fails except for the 1st interface
	if (bind(s.m_sd, (struct sockaddr *)&group_sock, sizeof(group_sock))) {
		ERR("Binding datagram socket error: %s\n", strerror(errno));
		//close(s.m_sd);
		//return -1;
	} else {
		DBG("Binding datagram socket...OK.\n");
    }

	/* Join the multicast group on the local */
	/* interface. Note that this IP_ADD_MEMBERSHIP option must be */
	/* called for each local interface over which the multicast */
	/* datagrams are to be received. */
    if(NULL!=mcastip) {
        memset(&s.m_groupreq, 0, sizeof(s.m_groupreq));
        s.m_groupreq.imr_multiaddr.s_addr = inet_addr(mcastip);
        s.m_groupreq.imr_interface.s_addr = inet_addr(s.m_if.ip);
        if (setsockopt
            (s.m_sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&s.m_groupreq,
             sizeof(s.m_groupreq)) < 0) {
            ERR("Adding multicast group error: %s\n", strerror(errno));
            close(s.m_sd);
            return -1;
        } else {
            DBG("Adding multicast group...OK.\n");
        }
    }

    m_sockets.push_back(s);
    return 0;
}

int 
UDPMCastReceiver::Init(std::vector<InetInterface> &ifaces, const char *mcastip, const short port)
{
    std::vector<InetInterface>::iterator it; 

    for(it=ifaces.begin(); it!=ifaces.end(); ++it) {

        int ret;
        ret = InitOne(*it, mcastip, port);
        if(ret!=0) {
            ERR("cannot init interface %s with %s:%d\n", it->ip, mcastip, port);
            return ret;
        }

    }
    return 0;
}
