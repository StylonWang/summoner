#ifndef __UDP_SENDER_H__
#define __UDP_SENDER_H__

class UDPSender 
{

    public:
        UDPSender();
        ~UDPSender();

        int Init(const char *destip, const short destport, bool nonblocking);

        ssize_t Send(const void *buffer, size_t length);

};

#endif //__UDP_SENDER_H__
