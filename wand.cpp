/*
 *
 * tool to find other devices on LAN.
 *
 * Author: wangstylon@gmail.com
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <vector>

#include "spells.h"

#define DBG(fmt, args...) do { fprintf(stdout, fmt, ##args ); } while(0)
#define ERR(fmt, args...) do { fprintf(stderr, fmt, ##args); } while(0)

struct interface {
    char name[16];
    char ip[INET_ADDRSTRLEN];

    struct sockaddr_in group_sock;
    int sd; // socket to UDP multicast

    // constructor
    interface(const char *n, const char *ipaddr)
    {
        strncpy(name, n, sizeof(name));
        strncpy(ip, ipaddr, sizeof(ip));
    }
};

static std::vector<interface> get_inet_interfaces(void)
{
    struct ifaddrs *ifap = NULL, *ifap_orig = NULL;
    int ret;
    std::vector<interface> result;

    ret = getifaddrs(&ifap);
    if(ret) return result;
    ifap_orig = ifap;

    while(ifap) {

        struct sockaddr_in *addr = (struct sockaddr_in *)ifap->ifa_addr;
        
        // skip other than IPV4
        if(AF_INET==addr->sin_family && strncmp(ifap->ifa_name, "lo", 2)) {

#if 0
            DBG("%s:%d %s, %s\n", 
                    (addr->sin_family==AF_INET)? "INET" :
                    (addr->sin_family==AF_INET6)? "INET6" :
                    "OTH",
                    addr->sin_family,
                    ifap->ifa_name, 
                    inet_ntoa(addr->sin_addr)
                    );
#endif
            result.push_back(interface(ifap->ifa_name, inet_ntoa(addr->sin_addr)));
        }

        ifap = ifap->ifa_next; //next interface
    }

    if(ifap_orig) freeifaddrs(ifap_orig);
    
    return result;
}

typedef struct _wand_t
{
    std::vector<interface> interfaces;
} wand_t;

int wand_init(wand_t *wt)
{
    std::vector<interface>::iterator it;

    memset(wt, 0, sizeof(*wt));

    // gather interface info: names and ip addresses
    wt->interfaces = get_inet_interfaces();

    // debug only
    for(it=wt->interfaces.begin(); it!=wt->interfaces.end(); ++it) {
       DBG("%s, %s\n", it->name, it->ip); 
    }

    // create multicast sockets, one for each interface
    DBG("creating sockets...\n");
    for(it=wt->interfaces.begin(); it!=wt->interfaces.end(); ++it) {

        it->sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        struct in_addr local_if;

        if(it->sd<0) {
            ERR("cannot create socket: %s\n", strerror(errno));
            return -1;
        }

        //TODO: will we need to do this on each send? (is this affecting this socket ony?)
        // assign this interface to send multicast packets
        local_if.s_addr = inet_addr(it->ip);
        if (0!=setsockopt
            (it->sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&local_if,
             sizeof(local_if))) {

            ERR("cannot set local interface: %s\n", strerror(errno));
            close(it->sd);
            return -1;
        }

        int loop = 1;
        if (0!=setsockopt(it->sd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop))) {
            ERR("cannot disable multicast loop: %s\n", strerror(errno));
            close(it->sd);
            return -1;
        }

        loop = 3;
        socklen_t size = -5;
        getsockopt(it->sd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, &size);
        DBG("multicast looping is %d, sz%d\n", loop, size);

        memset((char *)&it->group_sock, 0, sizeof(it->group_sock));
        it->group_sock.sin_family = AF_INET;
        it->group_sock.sin_addr.s_addr = inet_addr(SPELL_MULTICAST_IP);
        it->group_sock.sin_port = htons(SPELL_MULTICAST_PORT);
#if 0
        // set non-blocking mode
        int opt = 1;
        if(-1==ioctl(it->sd, FIONBIO, &opt)) {
            printf("cannot set to nonblock: %s\n", strerror(errno));
            close(it->sd);
            return -1;
        }
#endif //0
    }

    return 0;
}

void wand_finish(wand_t *wt)
{
    std::vector<interface>::iterator it;

    for(it=wt->interfaces.begin(); it!=wt->interfaces.end(); ++it) {
        close(it->sd);
    }
}

int wand_send_discovery(wand_t *wt)
{
    std::vector<interface>::iterator it;
    spell_t s;

    memset(&s, 0, sizeof(s));
    s.version = SPELL_VERSION;
    s.spell = SPELL_APARECIUM;

    for(it=wt->interfaces.begin(); it!=wt->interfaces.end(); ++it) {
        int ret = 0;

        DBG("sending APARECIUM...\n");
        ret = sendto(it->sd, &s, sizeof(s), 0, (struct sockaddr *)&it->group_sock,
                                  sizeof(it->group_sock));

        if(ret<0) {
            ERR("%s error: %s\n", __FUNCTION__, strerror(errno));
        }
        DBG("APARECIUM sent on %s\n", it->name);
    }
    return 0;
}

int wand_receive_replies(wand_t *wt)
{
    return 0;
}

int main(int argc, char **argv)
{
    int ret = 0;
    wand_t wt;

    ret = wand_init(&wt);
    if(ret<0) {
        goto error;    
    }

    ret = wand_send_discovery(&wt);
    if(ret<0) {
        goto error;    
    }

    ret = wand_receive_replies(&wt);
    if(ret<0) {
        goto error;    
    }

    wand_finish(&wt);
    return 0;

error:
    return -1;
}

