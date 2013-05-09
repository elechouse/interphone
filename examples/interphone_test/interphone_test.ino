/**
  @file    interphone_test.ino
  @author  www.elechouse.com
  @brief   example of interphone module
   
  @section  HISTORY
  V1.0 2012-12-25 initial version
  
    Copyright (c) 2012 www.elechouse.com  All right reserved.
*/
#include <interphone.h>

INTERPHONE phone;

u32 time;
u32 time_last=0;
u16 num=0;
u16 channel=0;
u16 channel_cur=0;

void setup(void)
{
  /** start interphone, initial Serial.begin(9600); */
  phone.begin();
  
  /** connect with interphone */
  phone.ATConnect();
  phone.ATReceive();
  
  /** Get version */
  phone.ATGetVersion();
  phone.ATReceive();
  
  /** set volume */
  phone.ATSetVolume(6);
  phone.ATReceive();
  
  /** disable AutoSavePower */
  phone.ATSavePower(false);
  phone.ATReceive();
}

void loop(void)
{
  uint8_t dir;
  /** nixie tube blink control and print received packet */
  phone.sta_handle();
  
  /** push the rotary encoder key to set channel */
  if(phone.scan_encoder_key()&0x01){
    phone.blink_ctl(false);
    if(channel_cur != channel){
      channel_cur = channel;
      phone.ATSetGroup(channel);
    }
  }
  
  /** rotary encoder handle */
  dir = phone.scan_encoder();
  if(dir==1){
    phone.blink_ctl(true);
    Serial.write("DIR1\r\n");
    channel++;
    if(channel == 10000){
      channel = 0;
    }
    phone.write(channel);
  }else if(dir==2){
    phone.blink_ctl(true);
    Serial.write("DIR2\r\n");
    channel--;
    if(channel == 0xFFFF){
      channel = 9999;
    }
    phone.write(channel);
  }
}
