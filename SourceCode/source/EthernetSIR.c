//Local Includes

#define LOG_MODULE  LOG_MENU_MODULE
#include <dev/board.h>
#include <sys/timer.h>

#include <stdint.h>
#include <stdio.h>
#include <io.h>

/*
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pro/sntp.h>

#include <pro/dhcp.h>
#include <pro/discover.h>
*/
#include <string.h>
#include <time.h>

#include "log.h"
#include "uart0driver.h"

#include "EthernetSIR.h"

//Local defs

#define MY_MAC  { 0x00, 0x06, 0x98, 0x35, 0x02, 0x76 }
#define MY_IP   "192.168.0.111"
#define MY_MASK "255.255.255.0"


//dev/board.h aanpassen!!!!!!

static NutDiscoveryCallback default_handler;
static int MyHandler(uint32_t ip, uint16_t port, DISCOVERY_TELE * dist, int len);

int initNetworkStatic(){
	int returnmsg;
	uint8_t mac[6] = MY_MAC;
	uint32_t ip_addr = inet_addr(MY_IP);
	uint32_t ip_mask = inet_addr(MY_MASK);
	
	if (NutRegisterDevice(&DEV_ETHER, 0, 0)) {
        //Registering failed
		returnmsg = 0;
    }
    // Configure network.
    else if (NutNetIfConfig(DEV_ETHER_NAME, mac, ip_addr, ip_mask)) {
        //Configuring failed
		returnmsg = 1;
    }
    // Done.
    else {
        //OK
		returnmsg = 2;
    }

	return returnmsg;
}

int initNetworkDHCP(){
	int retval;
	if (NutRegisterDevice(&DEV_ETHER, 0, 0)) {
		LogMsg_P(LOG_INFO, PSTR("Registering ethernet failed."));
		retval = 1;
    }
    else if (NutDhcpIfConfig(DEV_ETHER_NAME, NULL, 0)) {
        uint8_t initial_mac[6] = MY_MAC;
 
        if (NutDhcpIfConfig(DEV_ETHER_NAME, initial_mac, 0)) {
            u_long initial_ip_addr = inet_addr(MY_IP);
            u_long initial_ip_mask = inet_addr(MY_MASK);
            NutNetIfConfig(DEV_ETHER_NAME, initial_mac, initial_ip_addr, initial_ip_mask);
			retval = 2;
        }
    }
    if (NutRegisterDiscovery(INADDR_BROADCAST, 0, DISF_INITAL_ANN)) {
        //printf("Discover Service failed");
		LogMsg_P(LOG_INFO, PSTR("Discover Service failed"));
		retval = 3;
    } else {
		LogMsg_P(LOG_INFO, PSTR("Ethernet Startup Message: OK"));
		LogMsg_P(LOG_INFO, PSTR("IP = [%d],[%d],[%d],[%d]"));
        default_handler = NutRegisterDiscoveryCallback(MyHandler);
		retval = 4;
    }
	return retval;
}

static int MyHandler(uint32_t ip, uint16_t port, DISCOVERY_TELE * dist, int len)
{
    char * tt;
 
    switch (dist->dist_type) {
    case DIST_REQUEST:
        tt = "request";
        break;
    case DIST_ANNOUNCE:
        tt = "announce";
        break;
    case DIST_APPLY:
        tt = "apply";
        break;
    default:
        tt = "telegram";
        break;
    }
    //printf("Discovery %s from %s\n", tt, inet_ntoa(ip));
    return (*default_handler)(ip, port, dist, len);
}

//Haal tijd op via ntp
tm getNTPTime(){
 
    time_t ntp_time = 0;
    tm *ntp_datetime;
    uint32_t timeserver = 0;
 
    _timezone = -1 * 60 * 60; //GMT+1
 
	LogMsg_P(LOG_INFO, PSTR("Retrieving time from 78.192.65.63..."));
 
    timeserver = inet_addr("78.192.65.63");
 	int i;
    for (i = 0;i < 20; i++) {
        if (NutSNTPGetTime(&timeserver, &ntp_time) == 0) {
            break;
        } else {
            NutSleep(1000);
			LogMsg_P(LOG_INFO, PSTR("Fout bij ophalen ntp tijd, pogingen over:%d"),(20-i));
        }
    }
    ntp_datetime = localtime(&ntp_time);

	LogMsg_P(LOG_INFO, PSTR("NTP time is: %02d:%02d:%02d\n"), ntp_datetime->tm_hour, ntp_datetime->tm_min, ntp_datetime->tm_sec);
 
    //printf("NTP time is: %02d:%02d:%02d\n", ntp_datetime->tm_hour, ntp_datetime->tm_min, ntp_datetime->tm_sec);
 	tm gmt;
	gmt.tm_hour = ntp_datetime->tm_hour;
	gmt.tm_min  = ntp_datetime->tm_min;
	gmt.tm_sec	= ntp_datetime->tm_sec;
	gmt.tm_mday = ntp_datetime->tm_mday;
	gmt.tm_mon  = ntp_datetime->tm_mon;
	gmt.tm_year = ntp_datetime->tm_year;

	return gmt;
}

//Connect met tcp server

void clientConnect(TCPSOCKET *sock){
	//sock = NutTcpCreateSocket();
	if (NutTcpConnect(sock, inet_addr("173.194.67.94"), 80)) {
    LogMsg_P(LOG_INFO, PSTR("Fout bij connecten socket"));
	}
}

//Verzend data naar server

void clientSend(TCPSOCKET *sock, char* buffer, int len){
	if (NutTcpSend(sock, buffer, len) != len) {
    LogMsg_P(LOG_INFO, PSTR("Fout bij zenden client data"));
	}
	else{
		LogMsg_P(LOG_INFO, PSTR("Geen Fout bij zenden client data?"));
	}
}

//Ontvang data van server

int clientReceive(TCPSOCKET *sock, char *buffer, int len){
	int got;
	got = NutTcpReceive(sock, buffer, len);
	if (got <= 0) {
    LogMsg_P(LOG_INFO, PSTR("Fout bij ontvangen client data"));
	}
	return got;
}

//Sluit de connectie met de server

void clientClose(TCPSOCKET *sock){
	NutTcpCloseSocket(sock);
}
