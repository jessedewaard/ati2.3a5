#include <time.h>

#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pro/sntp.h>

#include <pro/dhcp.h>
#include <pro/discover.h>

#include <sys/socket.h>


int initNetworkStatic(void);
int initNetworkDHCP(void);
tm getNTPTime(void);
void clientConnect(TCPSOCKET *sock);
void clientSend(TCPSOCKET *sock, char* buffer, int len);
int clientReceive(TCPSOCKET *sock, char *buffer, int len);
void clientClose(TCPSOCKET *sock);
