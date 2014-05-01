#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "udpsender.h"

int main(int argc, char **argv)
{
    if(argc<3) {
        printf("Usage: %s dest-ip dest-port\n", argv[0]);
        return -1;
    }

    std::vector<InetInterface> interfaces;
    interfaces = InetInterface::getAllInterfaces();

    if(interfaces.size()==0) {
        printf("no local interface available\n");
    }

    UDPSender sender;
    int ret;
    char buf[] = "UDP hello world!\n";

    ret = sender.Init(interfaces[0], argv[1], atoi(argv[2]), true, false);
    if(ret<0) {
        printf("failed to init sender\n");
        return -1;
    }
    else {
       printf("sender init ok\n"); 
    }

    ret = sender.Send(buf, strlen(buf));
    printf("%d bytes sent\n", ret);
    
    return 0;
}

