#include <stdio.h>
#include <stdlib.h>

#include "udpmcastreceiver.h"
#include "spells.h"


int main(int argc, char **argv)
{
    if(argc<3) {
        printf("Usage: %s mcast_ip mcast_port\n", argv[0]);
        return -1;
    }

    std::vector<InetInterface> interfaces;
    interfaces = InetInterface::getAllInterfaces();

    if(interfaces.size()==0) {
        printf("no local interface available, abort\n");
        return -1;
    }

    UDPMCastReceiver receiver;
    int ret;

    //ret = receiver.Init(interfaces[0], argv[1], atoi(argv[2]));
    //WARNING: hard coding to 2nd interface
    ret = receiver.Init(interfaces[1], argv[1], atoi(argv[2]));
    if(ret<0) {
        printf("init failed!\n");
        return -1;
    }

    while(1) {
        char buf[64*1024];
        int timeout_in_ms = 1000; // 1000 mili second timeout
        
        ret = receiver.Receive(buf, sizeof(buf), timeout_in_ms);

        if(ret>0) {
            printf("received %d bytes: %x %x %x %x\n", ret, buf[0], buf[1], buf[2], buf[3]);
        }
        else if(ret<0) {
            printf("error %d\n", ret);
            break;
        }
        else if(ret==0) {
            printf("nothing\n");
        }
    }

    return 0;
}

