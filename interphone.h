/*****************************************************************************/
/*!
    @file     interphone.h
    @author   www.elechouse.com
	@brief      Interphone Shield driver library header file

    NOTE:
	  NONE.
	@section  HISTORY
    V1.0	2012-12-25	Initial version.

    Copyright (c) 2012 www.elechouse.com  All right reserved.
*/
/*****************************************************************************/
#ifndef __INTERPHONE_H
#define __INTERPHONE_H

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
#include <avr/pgmspace.h>

#include <SoftwareSerial.h>

#ifndef __TYPE_REDEFINE
#define __TYPE_REDEFINE
typedef uint8_t u8;
typedef int8_t  s8;
typedef uint16_t u16;
typedef int16_t  s16;
typedef uint32_t u32;
typedef int32_t  s32;
#endif

#define DS_POS0     (4)
#define DS_POS1     (5)

#define DS_SEGD     (6)
#define DS_SEGH     (7)
#define DS_SEGE     (8)
#define DS_SEGC     (9)
#define DS_SEGB     (10)
#define DS_SEGG     (11)
#define DS_SEGF     (12)
#define DS_SEGA     (13)


#define IP_PD       (A2)
#define IP_PD_H()   digitalWrite(IP_PD, HIGH)
#define IP_PD_L()   digitalWrite(IP_PD, LOW)

#define IP_SQ       (A1)

#define IP_ENCODER_A		(A5)
#define IP_ENCODER_B		(A4)
#define IP_ENCODER_KEY		(A3)

#define IP_ENCODER_PINA		(PINC&0x10)
#define IP_ENCODER_PINB		(PINC&0x20)

class INTERPHONE{
public:
    INTERPHONE(void);
    void begin(void);
    void sta_handle(void);
    void write(u16 num);
	u8 scan_encoder_key(void);
	u8 scan_encoder(void);
	void blink_ctl(bool sta);
    void ATConnect();
    void ATSetGroup(u16 channel, u8 sq=3);
    void ATSavePower(bool sta);
    void ATGetVersion();
    void ATSetVolume(u8 vol);
	void ATReceive();
	// void ATSetVOX();
	// void ATSetMIC();
	// void ATMessage();
private:
    void set_seg(u8 dt);
    void set_pos(u8 pos);
    unsigned long time;
    unsigned long time_last;
	unsigned char blink;
	u8 blink_cnt;
};

#endif/** __INTERPHONE_H */
