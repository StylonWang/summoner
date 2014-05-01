
#include "udpreceiver.h"
#include "sdebug.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>

int UDPReceiver::InitOne(const InetInterface &iface, const short port);

UDPReceiver::UDPReceiver()
{

}

UDPReceiver::~UDPReceiver()
{

}

int UDPReceiver::Init(std::vector<InetInterface> &ifaces, const short port)
{

}

// return number of bytes received, -1 if error, zero if no message
int UDPReceiver::Receive(char *buf, int buflen, int timeout_in_msec=0)
{

}

