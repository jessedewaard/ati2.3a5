/*! \mainpage SIR firmware documentation
 *
 *  \section intro Introduction
 *  A collection of HTML-files has been generated using the documentation in the sourcefiles to
 *  allow the developer to browse through the technical documentation of this project.
 *  \par
 *  \note these HTML files are automatically generated (using DoxyGen) and all modifications in the
 *  documentation should be done via the sourcefiles.
 */

/*! \file
 *  COPYRIGHT (C) STREAMIT BV 2010
 *  \date 19 december 2003
 */
 
 
 

#define LOG_MODULE  LOG_MAIN_MODULE

/*--------------------------------------------------------------------------*/
/*  Include files                                                           */
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/thread.h>
#include <sys/timer.h>
#include <sys/version.h>
#include <dev/irqreg.h>

#include <sys/socket.h>

#include "system.h"
#include "portio.h"
#include "display.h"
#include "remcon.h"
#include "keyboard.h"
#include "led.h"
#include "log.h"
#include "uart0driver.h"
#include "mmc.h"
#include "watchdog.h"
#include "flash.h"
#include "spidrv.h"

#import  "EthernetSIR.h"

#include <time.h>
#include "rtc.h"


/*-------------------------------------------------------------------------*/
/* global variable definitions                                             */
/*-------------------------------------------------------------------------*/
tm gmt;
tm pgmt;
char buffer [32] = "GET /search?q=arduino HTTP/1.0\n\n";
/*-------------------------------------------------------------------------*/
/* local variable definitions                                              */
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/* local routines (prototyping)                                            */
/*-------------------------------------------------------------------------*/
static void SysMainBeatInterrupt(void*);
static void SysControlMainBeat(u_char);
void TimeToBuffer(char*);

/*-------------------------------------------------------------------------*/
/* Stack check variables placed in .noinit section                         */
/*-------------------------------------------------------------------------*/

/*!
 * \addtogroup System
 */

/*@{*/


/*-------------------------------------------------------------------------*/
/*                         start of code                                   */
/*-------------------------------------------------------------------------*/


/* อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ */
/*!
 * \brief ISR MainBeat Timer Interrupt (Timer 2 for Mega128, Timer 0 for Mega256).
 *
 * This routine is automatically called during system
 * initialization.
 *
 * resolution of this Timer ISR is 4,448 msecs
 *
 * \param *p not used (might be used to pass parms from the ISR)
 */
/* อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ */

static void SysMainBeatInterrupt(void *p)
{

    /*
     *  scan for valid keys AND check if a MMCard is inserted or removed
     */
    KbScan();
    CardCheckCard();
}


void TimeToBuffer(char *buffer){
	int n = sprintf(buffer,"%02d:%02d:%02d", gmt.tm_hour, gmt.tm_min, gmt.tm_sec );
}


/* อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ */
/*!
 * \brief Initialise Digital IO
 *  init inputs to '0', outputs to '1' (DDRxn='0' or '1')
 *
 *  Pull-ups are enabled when the pin is set to input (DDRxn='0') and then a '1'
 *  is written to the pin (PORTxn='1')
 */
/* อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ */
void SysInitIO(void)
{
    /*
     *  Port B:     VS1011, MMC CS/WP, SPI
     *  output:     all, except b3 (SPI Master In)
     *  input:      SPI Master In
     *  pull-up:    none
     */
    outp(0xF7, DDRB);

    /*
     *  Port C:     Address bus
     */

    /*
     *  Port D:     LCD_data, Keypad Col 2 & Col 3, SDA & SCL (TWI)
     *  output:     Keyboard colums 2 & 3
     *  input:      LCD_data, SDA, SCL (TWI)
     *  pull-up:    LCD_data, SDA & SCL
     */
    outp(0x0C, DDRD);
    outp((inp(PORTD) & 0x0C) | 0xF3, PORTD);

    /*
     *  Port E:     CS Flash, VS1011 (DREQ), RTL8019, LCD BL/Enable, IR, USB Rx/Tx
     *  output:     CS Flash, LCD BL/Enable, USB Tx
     *  input:      VS1011 (DREQ), RTL8019, IR
     *  pull-up:    USB Rx
     */
    outp(0x8E, DDRE);
    outp((inp(PORTE) & 0x8E) | 0x01, PORTE);

    /*
     *  Port F:     Keyboard_Rows, JTAG-connector, LED, LCD RS/RW, MCC-detect
     *  output:     LCD RS/RW, LED
     *  input:      Keyboard_Rows, MCC-detect
     *  pull-up:    Keyboard_Rows, MCC-detect
     *  note:       Key row 0 & 1 are shared with JTAG TCK/TMS. Cannot be used concurrent
     */
#ifndef USE_JTAG
    sbi(JTAG_REG, JTD); // disable JTAG interface to be able to use all key-rows
    sbi(JTAG_REG, JTD); // do it 2 times - according to requirements ATMEGA128 datasheet: see page 256
#endif //USE_JTAG

    outp(0x0E, DDRF);
    outp((inp(PORTF) & 0x0E) | 0xF1, PORTF);

    /*
     *  Port G:     Keyboard_cols, Bus_control
     *  output:     Keyboard_cols
     *  input:      Bus Control (internal control)
     *  pull-up:    none
     */
    outp(0x18, DDRG);
}

/* อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ */
/*!
 * \brief Starts or stops the 4.44 msec mainbeat of the system
 * \param OnOff indicates if the mainbeat needs to start or to stop
 */
/* อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ */
static void SysControlMainBeat(u_char OnOff)
{
    int nError = 0;

    if (OnOff==ON)
    {
        nError = NutRegisterIrqHandler(&OVERFLOW_SIGNAL, SysMainBeatInterrupt, NULL);
        if (nError == 0)
        {
            init_8_bit_timer();
        }
    }
    else
    {
        // disable overflow interrupt
        disable_8_bit_timer_ovfl_int();
    }
}

THREAD(Thread1, arg)
{
	LcdSetCursorPosition(0,4);
	PrintStr("Ingedrukt");
	int licht = 1;

    for (;;) {
		LcdSetCursorPosition(1,11);
		PrintStr("\1");
		LcdSetCursorPosition(1,12);
		PrintStr("\2");
		LcdSetCursorPosition(1,13);
		PrintStr("\3");
		LcdSetCursorPosition(0,15);
		PrintStr("\4");
		LcdSetCursorPosition(1,15);
		PrintStr("\5");
        NutSleep(100);
		LcdSetCursorPosition(0,0);
		PrintStr("000");		

    	char buffer [16];
		int n;
		char kb = KbGetKey();
		if(kb < 125 && kb > 0){
			n = sprintf(buffer,"%d",kb);
			LcdSetCursorPosition(0,(3-n));
			PrintStr(buffer);
		}
		if(kb == 13){
			if(licht == 1){
				licht = 0;
			}else{
				licht = 1;
			}
			LcdBackLight(licht);
			
		}
		//PrintStr(sprintf("%s",KbGetKey()));
		NutSleep(100);

    }
}

/*
*	Update rtc time op display
*/
char rtctime [16];
THREAD(TimeUpdater, arg){
	for(;;){
		NutSleep(100);
		if (X12RtcGetClock(&gmt) == 0)
    	{
			TimeToBuffer(time);
    	}
		LcdBufferLoader(NULL,time);

	}
}



 

/* อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ */
/*!
 * \brief Main entry of the SIR firmware
 *
 * All the initialisations before entering the for(;;) loop are done BEFORE
 * the first key is ever pressed. So when entering the Setup (POWER + VOLMIN) some
 * initialisatons need to be done again when leaving the Setup because new values
 * might be current now
 *
 * \return \b never returns
 */
/* อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ */

	#define USE_JTAG
int main(void)
{
	
    WatchDogDisable();

    NutDelay(100);

    SysInitIO();
	
	SPIinit();
    
	LedInit();
	
	LcdLowLevelInit();
	LcdBackLight(1);
	PrintStr("Starting System");
    Uart0DriverInit();
    Uart0DriverStart();
	LogInit();

    CardInit();

	char custom[48] = ALL;
	
	LoadCustomChars(custom, 6);
	
    X12Init();
    if (X12RtcGetClock(&gmt) == 0)
    {
		LogMsg_P(LOG_INFO, PSTR("RTC time [%02d:%02d:%02d]"), gmt.tm_hour, gmt.tm_min, gmt.tm_sec );
    }

    if (At45dbInit()==AT45DB041B)
    {
        // ......
    }


    RcInit();
    
	KbInit();

    SysControlMainBeat(ON);             // enable 4.4 msecs hartbeat interrupt
	
	ClearLcdScreen();

	//Nieuwe threads aanmaken
    NutThreadCreate("t01", Thread1, NULL, 512);
	NutThreadCreate("time", TimeUpdater, NULL, 512);
	
	//Start netwerk
	int i = initNetworkDHCP();
	LogMsg_P(LOG_INFO, PSTR("Ethernet Startup Message: [%d]"),i);

	//Haal Internet tijd op
	pgmt = getNTPTime();
	X12RtcSetClock(&pgmt);
	//LogMsg_P(LOG_INFO, PSTR("New RTC time [%02d:%02d:%02d]"), gmt.tm_hour, gmt.tm_min, gmt.tm_sec );

    /*
     * Increase our priority so we can feed the watchdog.
     */
    NutThreadSetPriority(1);

	/* Enable global interrupts */
	sei();

	/******************NETWERK**TEST*****************************************************/

	//Maak nieuwe socket
	TCPSOCKET *sock;
	sock = NutTcpCreateSocket();
	//Connect met google.nl
	LogMsg_P(LOG_INFO, PSTR("Connecting client"));
	clientConnect(sock);
	//Zend http req
	clientSend(sock,buffer,sizeof(buffer));

	//Ontvang response in buffer --> grotere buffer (1500)= crash-->heapsize??(8k ram)
	char rcvbuffer [500];
	int rec;
	rec = clientReceive(sock,rcvbuffer,sizeof(rcvbuffer));
	LogMsg_P(LOG_INFO, PSTR("received [%d]"),rec);

	//0 terminate buffer(string)
	//rcvbuffer[499] = 0;

	//Print buffer(http response enz)
	LogMsg_P(LOG_INFO, PSTR("received [%s]"),rcvbuffer);
	
	//Sluit connectie
	clientClose(sock);
	/***********************************************************************************/

	//Main gui besturing??
    for (;;)
    {
        NutSleep(100);
        WatchDogRestart();
    }

    return(0);      
}

