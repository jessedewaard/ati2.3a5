/* ========================================================================
 * [PROJECT]    SIR100
 * [MODULE]     Display
 * [TITLE]      display header file
 * [FILE]       display.h
 * [VSN]        1.0
 * [CREATED]    030414
 * [LASTCHNGD]  030414
 * [COPYRIGHT]  Copyright (C) STREAMIT BV 2010
 * [PURPOSE]    API and gobal defines for display module
 * ======================================================================== */

#ifndef _Display_H
#define _Display_H


/*-------------------------------------------------------------------------*/
/* global defines                                                          */
/*-------------------------------------------------------------------------*/
#define DISPLAY_SIZE                16
#define NROF_LINES                  2
#define MAX_SCREEN_CHARS            (NROF_LINES*DISPLAY_SIZE)

#define LINE_0                      0
#define LINE_1                      1

#define FIRSTPOS_LINE_0             0
#define FIRSTPOS_LINE_1             0x40


#define LCD_BACKLIGHT_ON            1
#define LCD_BACKLIGHT_OFF           0

#define ALL_ZERO          			0x00      // 0000 0000 B
#define WRITE_COMMAND     			0x02      // 0000 0010 B
#define WRITE_DATA        			0x03      // 0000 0011 B
#define READ_COMMAND      			0x04      // 0000 0100 B
#define READ_DATA         			0x06      // 0000 0110 B

#define INET_CONNECTED_SYMBOL		138,138,159,159,142,132,132,132
#define WEKKER_AAN					142,149,149,149,151,145,145,142
#define NTP_SYNC					128,130,159,130,136,159,136,128
#define ARROW_UP					128,128,132,142,159,128,128,128
#define ARROW_DOWN					128,128,128,159,142,132,128,128

#define ALL	{138,138,159,159,142,132,132,132,138,138,159,159,142,132,132,132,142,149,149,149,151,145,145,142,128,130,159,130,136,159,136,128,128,128,132,142,159,128,128,128,128,128,128,159,142,132,128,128}

/*-------------------------------------------------------------------------*/
/* typedefs & structs                                                      */
/*-------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  Global variables                                                        */
/*--------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* export global routines (interface)                                      */
/*-------------------------------------------------------------------------*/
static void LcdWriteByte(u_char, u_char);
extern void LcdChar(char);
extern void LcdBackLight(u_char);
extern void LcdInit(void);
extern void LcdLowLevelInit(void);
void LcdSetCursorPosition(char, char);
void ClearLcdScreen();
void PrintStr(char*);
void BufferLoader(char*, char*);
void LoadCustomChars(char *, char);

#endif /* _Display_H */
/*  様様  End Of File  様様様様 様様様様様様様様様様様様様様様様様様様様様様 */





