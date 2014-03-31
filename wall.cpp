/*
 * Daemon to reply to finder requests.
 *
 *
 *
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
#include <sys/select.h>
#include <signal.h>

#include "spells.h"

#define DBG(fmt, args...) do { fprintf(stdout, fmt, ##args ); } while(0)
#define ERR(fmt, args...) do { fprintf(stderr, fmt, ##args); } while(0)

static char *get_local_inet_ip(void)
{
    struct ifaddrs *ifap = NULL, *ifap_orig = NULL;
    int ret;
    static char ip[INET_ADDRSTRLEN] = "";

    ret = getifaddrs(&ifap);
    if(ret) return NULL;
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
            snprintf(ip, sizeof(ip), "%s", inet_ntoa(addr->sin_addr));
            break; // found
        }

        ifap = ifap->ifa_next; //next interface
    }

    if(ifap_orig) freeifaddrs(ifap_orig);

    return ip;
}

typedef struct _service_t {

	struct sockaddr_in local_sock;
	struct ip_mreq group;
	int sd;

    char ip[INET_ADDRSTRLEN];

    int can_stop;

} service_t;

int service_init(service_t *t)
{
    char *local_ip = get_local_inet_ip();

    memset(t, 0, sizeof(*t));
    t->can_stop = 0;

    DBG("local ip %s\n", local_ip);
    snprintf(t->ip, sizeof(t->ip), "%s", local_ip);

	/* Create a datagram socket on which to receive. */
	t->sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (t->sd < 0) {
		DBG("Opening datagram socket error: %s\n", strerror(errno));
		return -1;
	} else
		DBG("Opening datagram socket....OK.\n");

	/* Enable SO_REUSEADDR to allow multiple instances of this */
	/* application to receive copies of the multicast datagrams. */
    int reuse = 1;
    if (setsockopt
        (t->sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse,
         sizeof(reuse)) < 0) {
        ERR("Setting SO_REUSEADDR error: %s\n", strerror(errno));
        close(t->sd);
        return -1;
    } else
        DBG("Setting SO_REUSEADDR...OK.\n");

	/* Bind to the proper port number with the IP address */
	/* specified as INADDR_ANY. */
	t->local_sock.sin_family = AF_INET;
	t->local_sock.sin_port = htons(SPELL_MULTICAST_PORT);
	t->local_sock.sin_addr.s_addr = INADDR_ANY;
	if (bind(t->sd, (struct sockaddr *)&t->local_sock, sizeof(t->local_sock))) {
		ERR("Binding datagram socket error: %s\n", strerror(errno));
		close(t->sd);
		return -1;
	} else
		DBG("Binding datagram socket...OK.\n");

	/* Join the multicast group 226.1.1.1 on the local 203.106.93.94 */
	/* interface. Note that this IP_ADD_MEMBERSHIP option must be */
	/* called for each local interface over which the multicast */
	/* datagrams are to be received. */
	t->group.imr_multiaddr.s_addr = inet_addr(SPELL_MULTICAST_IP);
	t->group.imr_interface.s_addr = inet_addr(local_ip);
	if (setsockopt
	    (t->sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&t->group,
	     sizeof(t->group)) < 0) {
		ERR("Adding multicast group error: %s\n", strerror(errno));
		close(t->sd);
		return -1;
	} else
		DBG("Adding multicast group...OK.\n");

    return 0;
}

void service_finish(service_t *t)
{
    if( setsockopt(t->sd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&t->group, sizeof(t->group)) < 0) {
        ERR("Dropping multicast group error: %s\n", strerror(errno));
    }
    close(t->sd);
}

void service_stop(service_t *t)
{
    t->can_stop = 1;
}

static int service_aparecium_reply(service_t *t, spell_t *sp)
{
    int sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct in_addr local_if;
    int ret;

    if(sd<0) {
        ERR("cannot create socket: %s\n", strerror(errno));
        return -1;
    }

    //TODO: will we need to do this on each send? (is this affecting this socket ony?)
    // assign this interface to send multicast packets
    local_if.s_addr = inet_addr(t->ip);
    if (0!=setsockopt
        (sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&local_if,
         sizeof(local_if))) {

        ERR("cannot set local interface: %s\n", strerror(errno));
        close(sd);
        return -1;
    }

    int loop = 1;
    if (0!=setsockopt(sd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop))) {
        ERR("cannot disable multicast loop: %s\n", strerror(errno));
        close(sd);
        return -1;
    }

    loop = 3;
    socklen_t size = -5;
    getsockopt(sd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, &size);
    DBG("multicast looping is %d, sz%d\n", loop, size);

    struct sockaddr_in group_sock;
    memset((char *)&group_sock, 0, sizeof(group_sock));
    group_sock.sin_family = AF_INET;
    group_sock.sin_addr.s_addr = inet_addr(SPELL_MULTICAST_IP);
    group_sock.sin_port = htons(SPELL_MULTICAST_PORT);

    spell_t s;

    memset(&s, 0, sizeof(s));
    s.version = SPELL_VERSION;
    s.spell = SPELL_APARECIUM_REPLY;

    DBG("sending APARECIUM REPLY...\n");
    ret = sendto(t->sd, &s, sizeof(s), 0, (struct sockaddr *)&group_sock,
                              sizeof(group_sock));

    if(ret<0) {
        ERR("%s error: %s\n", __FUNCTION__, strerror(errno));
    }
    DBG("APARECIUM REPLY sent\n");

    return 0;
}

static void handle_spell(service_t *t, spell_t *sp)
{
    // ignore incompatible version
    if(SPELL_VERSION!=sp->version) return;

    switch(sp->spell) {

    case SPELL_APARECIUM:
        service_aparecium_reply(t, sp);
        break;

    default:

        break;
    }
}

void service_serve(service_t *t)
{
	/* Read from the socket. */
    while(!t->can_stop) {
		struct sockaddr_in srcaddr;
		int srcaddrlen = sizeof(struct sockaddr_in);
		int ret = 0;
	    int datalen;
        spell_t spell;

        fd_set rset;
        struct timeval timeout;

        FD_ZERO(&rset);
        FD_SET(t->sd, &rset);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        ret = select(t->sd+1, &rset, NULL, NULL, &timeout);
        if(ret<0) {
            ERR("select failed: %s\n", strerror(errno));
            break;
        }
        else if(0==ret || !FD_ISSET(t->sd, &rset)) {
            continue;
        }

		datalen = sizeof(spell);
		memset(&srcaddr, 0, sizeof(srcaddr));
		ret = recvfrom(t->sd, &spell, datalen, 0, 
				(struct sockaddr *)&srcaddr, (socklen_t *)&srcaddrlen);
		if(ret < 0) {
			ERR("Reading datagram message error: %s\n", strerror(errno));
		}
		else {
			DBG("The message from  %s is: \"%d:%d\"\n",
				   inet_ntoa(srcaddr.sin_addr),
                   spell.version,
                   spell.spell
               );
		}

        handle_spell(t, &spell);
    } // end of while loop

    DBG("service stopped\n");
}


service_t g_t;

static void service_sig_handler(int signo)
{
    DBG("Quit\n");
    service_stop(&g_t);
}

int main(int argc, char **argv)
{
    int ret;

    if(0!=geteuid()) {
        ERR("Must have super user privilege, abort!\n");
        goto error;
    }

    signal(SIGINT, service_sig_handler);

    ret = service_init(&g_t);
    if(ret<0) {
        goto error;
    }

    service_serve(&g_t);

    service_finish(&g_t);

    return 0;
error:
    return -1;
}
