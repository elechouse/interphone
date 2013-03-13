/*****************************************************************************/
/*!
    @file     interphone.cpp
    @author   www.elechouse.com
	@brief      Interphone Shield driver library source file

    NOTE:
	  NONE.
	@section  HISTORY
    V1.0	2012-12-25	Initial version.

    Copyright (c) 2012 www.elechouse.com  All right reserved.
*/
/*****************************************************************************/
#include "interphone.h"

/** io sequence table for Nixie tube */
static u8 ds_seg[] = {
    DS_SEGA,DS_SEGB,DS_SEGC,DS_SEGD,
    DS_SEGE,DS_SEGF,DS_SEGG,DS_SEGH,
};

/** coding table for Nixie tube */
static u8 seg_tab[] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00
};

/** pattern display table */
u8 ds_dt[5]={};
u8 ds_dt_bak[5]={};

/** command buffer for controlling interphone module */
u8 cmd[64]="AT+DMO";

u8 dir = 0;

/** interphone module control port */
SoftwareSerial at(2,3);

/*****************************************************************************/
/*!
	@brief	write data to Nixie tube data bus
	@param	dt - data pattern
*/
/*****************************************************************************/
void set_seg(u8 dt)
{
    u8 i;
    for(i=0; i<8; i++){
        if(dt&0x01){
            digitalWrite(ds_seg[i], HIGH);
        }else{
            digitalWrite(ds_seg[i], LOW);
        }
        dt>>=1;
    }
}

/*****************************************************************************/
/*!
	@brief  turn on the specified nixie tube
	@param  pos - position of the specified nixie tube.
	@return NONE.
*/
/*****************************************************************************/
void set_pos(u8 pos)
{
    switch(pos){
        case 1:
            digitalWrite(DS_POS0, HIGH);
            digitalWrite(DS_POS1, HIGH);
            break;
        case 2:
            digitalWrite(DS_POS0, LOW);
            digitalWrite(DS_POS1, LOW);
            break;
        case 3:
            digitalWrite(DS_POS0, LOW);
            digitalWrite(DS_POS1, HIGH);
            break;
        case 4:
            digitalWrite(DS_POS0, HIGH);
            digitalWrite(DS_POS1, LOW);
            break;
        default:
            break;
    }
}

/*****************************************************************************/
/*!
	@brief	timer2 interrupt routine, execute every 2ms
	@param	NONE
	@return	NONE
*/
/*****************************************************************************/
SIGNAL(TIMER2_COMPA_vect)
{
	static u8 pos;
	
	set_seg(0x00);
	set_pos(pos);
	set_seg(seg_tab[ds_dt[pos]]);

	pos++;
	if(pos==5){
		pos=1;
	}
}

ISR(PCINT1_vect)
{
	delayMicroseconds(300);
	if(digitalRead(IP_ENCODER_A)==LOW){
		if(digitalRead(IP_ENCODER_B)==LOW){
			dir = 0x01;
		}else{
			dir = 0x02;
		}
	}
	
	// if(digitalRead(IP_ENCODER_B)==LOW){
		// if(digitalRead(IP_ENCODER_A)==LOW){
			// dir = 0x02;
		// }
	// }
}

/*****************************************************************************/
/*!
	@brief	constructor
	@param	NONE.
	@return	NONE.
*/
/*****************************************************************************/
INTERPHONE::INTERPHONE(void)
{
    time_last = millis();
	blink = 0;
	blink_cnt = 0;
}

/*****************************************************************************/
/*!
	@brief	start function of arduino format.
	@param	NONE.
	@return	NONE.
*/
/*****************************************************************************/
void INTERPHONE::begin(void)
{
	/** set mode and prescaler*/
	TCCR2A = 0x02;
	TCCR2B = 0x07;
	/** 16MHz count 2ms */
	TCNT2 = 0;
	OCR2A = 50;
	/** enable interrupt */
	TIMSK2 = 0x02;
	
    at.begin(9600);
    Serial.begin(9600);

    pinMode(DS_SEGA, OUTPUT);
    pinMode(DS_SEGB, OUTPUT);
    pinMode(DS_SEGC, OUTPUT);
    pinMode(DS_SEGD, OUTPUT);
    pinMode(DS_SEGE, OUTPUT);
    pinMode(DS_SEGF, OUTPUT);
    pinMode(DS_SEGG, OUTPUT);
    pinMode(DS_SEGH, OUTPUT);

    pinMode(DS_POS0, OUTPUT);
    pinMode(DS_POS1, OUTPUT);

    pinMode(IP_PD, OUTPUT);
	
	digitalWrite(IP_ENCODER_A, HIGH);
	digitalWrite(IP_ENCODER_B, HIGH);
	digitalWrite(IP_ENCODER_KEY, HIGH);
	pinMode(IP_ENCODER_A, INPUT);
	pinMode(IP_ENCODER_B, INPUT);
	pinMode(IP_ENCODER_KEY, INPUT);

	
	// *digitalPinToPCICR(IP_ENCODER_A) |= _BV(digitalPinToPCICRbit(IP_ENCODER_A));
    // *digitalPinToPCMSK(IP_ENCODER_A) |= _BV(digitalPinToPCMSKbit(IP_ENCODER_A));
	// *digitalPinToPCICR(IP_ENCODER_B) |= _BV(digitalPinToPCICRbit(IP_ENCODER_B));
    // *digitalPinToPCMSK(IP_ENCODER_B) |= _BV(digitalPinToPCMSKbit(IP_ENCODER_B));
	
	/** interphone module power on */
    IP_PD_H();

    set_seg(0x00);
	write(0);
	
	delay(500);
}

/** unused */
void INTERPHONE::set_seg(u8 dt)
{
    u8 i;
    for(i=0; i<8; i++){
        if(dt&0x01){
            digitalWrite(ds_seg[i], HIGH);
        }else{
            digitalWrite(ds_seg[i], LOW);
        }
        dt>>=1;
    }
}

/** unused */
void INTERPHONE::set_pos(u8 pos)
{
    switch(pos){
        case 1:
            digitalWrite(DS_POS0, HIGH);
            digitalWrite(DS_POS1, HIGH);
            break;
        case 2:
            digitalWrite(DS_POS0, LOW);
            digitalWrite(DS_POS1, LOW);
            break;
        case 3:
            digitalWrite(DS_POS0, LOW);
            digitalWrite(DS_POS1, HIGH);
            break;
        case 4:
            digitalWrite(DS_POS0, HIGH);
            digitalWrite(DS_POS1, LOW);
            break;
        default:
            break;
    }
}

/*****************************************************************************/
/*!
	@brief	puts received character from interphone module through Serial
	@param	NONE.
	@return	NONE.
*/
/*****************************************************************************/
void INTERPHONE::sta_handle(void)
{
	/** blink */
	if(blink){
		time = millis();
		if(time-time_last>300){
			time_last = time;
			if(blink_cnt){
				blink_cnt = 0;
				ds_dt[1] = 10;
				ds_dt[2] = 10;
				ds_dt[3] = 10;
				ds_dt[4] = 10;
			}else{
				blink_cnt = 1;
				memcpy(ds_dt, ds_dt_bak, 5);
			}
		}
	}
	
	/** print received character */
    if(at.available()){
        Serial.write(at.read());
    }
}

/*****************************************************************************/
/*!
	@brief	rotary encoder driver
	@param	NONE.
	@return	0 - no operation
			1 - clockwise
			2 - anti-clockwise
*/
/*****************************************************************************/
u8 INTERPHONE::scan_encoder(void)
{
	// u8 dir_tmp=0;
	// if(dir){
	// *digitalPinToPCMSK(IP_ENCODER_A) &= ~_BV(digitalPinToPCMSKbit(IP_ENCODER_A));
	// dir_tmp = dir;
	// dir = 0;
	// *digitalPinToPCMSK(IP_ENCODER_A) |= _BV(digitalPinToPCMSKbit(IP_ENCODER_A));
	// }
	// return dir_tmp;
	
	static u8 last_b,cur_b;
	u8 update=0, dir=0;
	if( (digitalRead(IP_ENCODER_A)==HIGH)&&(digitalRead(IP_ENCODER_B)==HIGH) ){
		return 0;
	}
	last_b = digitalRead(IP_ENCODER_B);
	while(digitalRead(IP_ENCODER_A)==LOW){
		cur_b = digitalRead(IP_ENCODER_B);
		update = 1;
	}
	if(update){
		update = 0;
		if( (last_b==LOW)&&(cur_b==HIGH) ){
			dir = 1;
		}else if( (last_b==HIGH)&&(cur_b==LOW) ){
			dir = 2;
		}
		// if(cur_b == LOW){
			// dir = 1;
		// }else{
			// dir = 2;
		// }
	}
	return dir;
}

/*****************************************************************************/
/*!
	@brief	rotary encoder button driver
	@param	NONE
	@return	0 - no operation
			1 - key push down
*/
/*****************************************************************************/
u8 INTERPHONE::scan_encoder_key(void)
{
	static u8 key_sta=0;
	u8 key;
	key = 0;
	switch(key_sta){
		case 0:
			if(digitalRead(IP_ENCODER_KEY)==LOW){
				key_sta = 1;
			}
			break;
		case 1:
			if(digitalRead(IP_ENCODER_KEY)==LOW){
				key_sta = 2;
				key |= 0x01;
			}
			break;
		case 2:
			if(digitalRead(IP_ENCODER_KEY)==HIGH){
				key_sta = 0;
			}
			break;
		default:
			key_sta = 0;
			break;
	}
	return key;
}

/*****************************************************************************/
/*!
	@brief	write data to nixie tube
	@param	dt - data to display (range 0-9999)
	@return	NONE
*/
/*****************************************************************************/
void INTERPHONE::write(u16 dt)
{
    u8 i;
    if(dt>9999){
        dt = 9999;
    }
    ds_dt[1] = dt/1000;
    ds_dt[2] = dt%1000/100;
    ds_dt[3] = dt%100/10;
    ds_dt[4] = dt%10;
    for(i=1; i<4; i++){
        if(ds_dt[i]){
            break;
        }else{
            ds_dt[i]=10;
        }
    }
	memcpy(ds_dt_bak, ds_dt, 5);
}

/*****************************************************************************/
/*!
	@brief	rotary encoder button driver
	@param	sta - "true" for enable blink, "false" for close blink
	@return	NONE
*/
/*****************************************************************************/
void INTERPHONE::blink_ctl(bool sta)
{
	if(sta == true){
		if(!blink){
			blink = 1;
			time_last = millis();
		}
	}else{
		if(blink){
			blink = 0;
			blink_cnt = 0;
			memcpy(ds_dt, ds_dt_bak, 5);
		}
	}
}

/*****************************************************************************/
/*!
	@brief	connect with interphone module
	@param	NONE
	@return	NONE
*/
/*****************************************************************************/
void INTERPHONE::ATConnect()
{
	at.write("AT+DMOCONNECT\r\n");
	
	TCCR2B = 0;
	delay(100);
	TCCR2B = 0x07;
}

/*****************************************************************************/
/*!
	@brief	select communication channel.
	@param	channel - channel to set, sq - unused, leave it to default.
	@return	NONE
*/
/*****************************************************************************/
void INTERPHONE::ATSetGroup(u16 channel, u8 sq)
{
	double ch_tmp;
	ch_tmp = 400+0.005*channel;
	//"AT+DMOSETGROUP=0,450.0250,450.0250,1,2,1"
	memcpy(cmd+6,"SETGROUP=",9);
	/** GBW */
	cmd[6+9]='0';
	cmd[6+10]=',';
	/** TFV */
	dtostrf(ch_tmp, 8, 4, (char *)cmd+6+11);
	cmd[6+11+8]=',';
	/** RFV */
	memcpy(cmd+6+11+9, cmd+6+11, 9);
	/** RXCXCSS */
	cmd[6+11+9+9] = '1';
	cmd[6+11+9+10] = ',';
	/** SQ */
	cmd[6+11+9+11] = 0x30+sq;
	cmd[6+11+9+12] = ',';
	/** TXCXCSS */
	cmd[6+11+9+13] = '1';
	cmd[6+11+9+14] = '\r';
	cmd[6+11+9+15] = '\n';
	cmd[6+11+9+16] = '\0';
	
	//Serial.write(cmd, 6+11+9+16);
	at.write(cmd, 6+11+9+16);
	TCCR2B = 0;
	delay(100);
	TCCR2B = 0x07;
}

/*****************************************************************************/
/*!
	@brief	enable or disable auto save power function
	@param	sta - ture - enable auto save power, 
						 false - disable auto save power
	@return	NONE
*/
/*****************************************************************************/
void INTERPHONE::ATSavePower(bool sta)
{
	memcpy(cmd+6,"AUTOPOWCONTR=",13);
	
	if(sta){
		cmd[6+13] = '0';
	}else{
		cmd[6+13] = '1';
	}
	
	cmd[6+14] = '\r';
	cmd[6+15] = '\n';
	cmd[6+16] = '\0';
	
	// Serial.write(cmd, 6+16);
	at.write(cmd, 6+16);
	TCCR2B = 0;
	delay(100);
	TCCR2B = 0x07;
}

/*****************************************************************************/
/*!
	@brief	get interphone module version
	@param	NONE
	@return	NONE
*/
/*****************************************************************************/
void INTERPHONE::ATGetVersion()
{
	memcpy(cmd+6,"VERQ ",4);
	cmd[6+4] = '\r';
	cmd[6+5] = '\n';
	cmd[6+6] = '\0';

	// Serial.write(cmd, 6+6);
	at.write(cmd, 6+6);
	TCCR2B = 0;
	delay(100);
	TCCR2B = 0x07;
}

/*****************************************************************************/
/*!
	@brief	set interphone module volume
	@param	volume to set
	@return	NONE
*/
/*****************************************************************************/
void INTERPHONE::ATSetVolume(u8 vol)
{
	memcpy(cmd+6,"SETVOLUME=",10);
	if(vol>9){
		vol = 9;
	}
	if(vol == 0){
		vol = 1;
	}
	cmd[6+10] = 0x30+vol;
	cmd[6+11] = '\r';
	cmd[6+12] = '\n';
	cmd[6+13] = '\0';
	
	//Serial.write(cmd, 6+13);
	at.write(cmd, 6+13);
	TCCR2B = 0;
	delay(100);
	TCCR2B = 0x07;
}

/*****************************************************************************/
/*!
	@brief	recieve data from interphone module and print through Serial.
	@param	NONE
	@return	NONE
*/
/*****************************************************************************/
void INTERPHONE::ATReceive()
{
    while(at.available()){
        Serial.write(at.read());
    }
}