#include "udpmcastsender.h"
#include "udpmcastreceiver.h"
#include "inetinterface.h"

#include <pthread.h>

// thread routine
static void *ssdp_receiver_thread(void *data)
{
    UDPMCastReceiver *receiver = reinterpret_cast<UDPMCastReceiver *>(data);
    int ret;
    char buf[5*1024];

    while(1) {
        memset(buf, 0, sizeof(buf));
        ret = receiver->Receive(buf, sizeof(buf), 500);

        if(ret<0) {
            printf("receive error %d\n", ret);
            return 0;
        }
        if(ret) {
            buf[ret] = 0;
            printf("received %d bytes: \n%s\n", ret, buf);
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    std::vector<InetInterface> interfaces;
    interfaces = InetInterface::getAllInterfaces();
    int ret;

    printf("Create receiver\n");
    // create receiver object
    UDPMCastReceiver receiver;
    ret = receiver.Init(interfaces, "239.255.255.250", 1900);
    if(ret<0) {
        printf("receiver init failed!\n");
        return -1;
    }

    // launch a thread to receive SSDP replies
    printf("Launch receiver thread\n");
    pthread_t t;
    ret = pthread_create(&t, NULL, ssdp_receiver_thread, reinterpret_cast<void *>(&receiver));
    if(ret!=0) {
        printf("cannot create thread: %s\n", strerror(ret));
        return -1;
    }

    // send SSDP request to every interfaces
    std::vector<InetInterface>::iterator it;
    for(it=interfaces.begin(); it!=interfaces.end(); ++it) {
        UDPMCastSender sender;
        char buf[] = "M-SEARCH * HTTP/1.1\r\nST: urn:schemas-avermedia-com:avercaster:1\r\nMX: 10\r\nMAN: \"ssdp:discover\"\r\nHOST: 239.255.255.250:1900\r\n\r\n";
        ssize_t ret;
        int iret;


        printf("sender init on interface %s, IP %s\n", it->name, it->ip);
        iret = sender.Init(*it, "239.255.255.250", 1900, false, true);

        printf("sender init %d\n", iret);

        ret = sender.Send(buf, sizeof(buf));

        printf("sending interfaces %ld\n", ret);
    }

}

