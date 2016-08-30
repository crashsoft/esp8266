/*
  PS2 mouse library
  Call first mouse_init();
*/


#ifdef __AVR__
 #include <avr/pgmspace.h>
#elif defined(ESP8266)
 #include <pgmspace.h>
#endif
#include "ps2.h"


void PS2_class::gohi(const int &pin)
{
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH);
}


void PS2_class::golo(const int &pin)
{
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}


void PS2_class::mouse_write(char data)
{
  if (!mouse_available) return;
  char i;
  char parity = 1;  
  /* put pins in output mode */
  gohi(_ps2data);
  gohi(_ps2clk);
  delayMicroseconds(300);
  golo(_ps2clk);
  delayMicroseconds(300);
  golo(_ps2data);
  delayMicroseconds(10);
  /* start bit */
  gohi(_ps2clk);
  
  /* wait for mouse to take control of clock); */
  while (digitalRead(_ps2clk) == HIGH);
    
  /* clock is low, and we are clear to send data */
  for (i=0; i < 8; i++) {
    if (data & 0x01) 
          gohi(_ps2data);    
    else  golo(_ps2data);
    
    /* wait for clock cycle */
    while (digitalRead(_ps2clk) == LOW);
    while (digitalRead(_ps2clk) == HIGH);
    parity = parity ^ (data & 0x01);
    data = data >> 1;
  }  
  /* parity */
  if (parity)
         gohi(_ps2data);  
  else   golo(_ps2data);
    
  while (digitalRead(_ps2clk) == LOW);
  while (digitalRead(_ps2clk) == HIGH);
  /* stop bit */
  gohi(_ps2data);
  delayMicroseconds(50);
  while (digitalRead(_ps2clk) == HIGH);
  /* wait for mouse to switch modes */
  while ((digitalRead(_ps2clk) == LOW) || (digitalRead(_ps2data) == LOW));
  /* put a hold on the incoming data. */
  golo(_ps2clk);    
}


/* Get a byte of data from the mouse */
char PS2_class::mouse_read(void)
{
  if (!mouse_available) return 0;
  int i;
  char data = 0x00;  
  char bit = 0x01;
  
  /* start the clock */
  gohi(_ps2clk);
  gohi(_ps2data);
  delayMicroseconds(50);
  while (digitalRead(_ps2clk) == HIGH);
  delayMicroseconds(5);  /* not sure why */
  while (digitalRead(_ps2clk) == LOW); /* eat start bit */
    
  for (i=0; i < 8; i++) {
    while (digitalRead(_ps2clk) == HIGH);
    if (digitalRead(_ps2data) == HIGH) {
      data = data | bit;
    }
    while (digitalRead(_ps2clk) == LOW);
    bit = bit << 1;
  }
  /* eat parity bit, which we ignore */
  while (digitalRead(_ps2clk) == HIGH);
  while (digitalRead(_ps2clk) == LOW);
  /* eat stop bit */
  while (digitalRead(_ps2clk) == HIGH);
  while (digitalRead(_ps2clk) == LOW);
  /* put a hold on the incoming data. */
  golo(_ps2clk);
  return data;
}

bool PS2_class::mouse_init(){
  char mouseId;
  
  if (!IsResponding())
     return false;
   
  gohi(_ps2clk);
  gohi(_ps2data);    
  mouse_write(0xff);
  mouse_read();  /* ack byte */  
  mouse_read();  /* blank */
  mouse_read();  /* blank */
  //  Serial.print("Setting sample rate 200\n");
  mouse_write(0xf3);  /* Set rate command */
  mouse_read();  /* ack */
  mouse_write(0xC8);  /* Set rate command */
  mouse_read();  /* ack */
  //  Serial.print("Setting sample rate 100\n");
  mouse_write(0xf3);  /* Set rate command */
  mouse_read();  /* ack */
  mouse_write(0x64);  /* Set rate command */
  mouse_read();  /* ack */
  //  Serial.print("Setting sample rate 80\n");
  mouse_write(0xf3);  /* Set rate command */
  mouse_read();  /* ack */
  mouse_write(0x50);  /* Set rate command */
  mouse_read();  /* ack */
  //  Serial.print("Read device type\n");
  mouse_write(0xf2);  /* Set rate command */
  mouse_read();  /* ack */
  mouse_read();  /* mouse id, if this value is 0x00 mouse is standard, if it is 0x03 mouse is Intellimouse */
  //  Serial.print("Setting wheel\n");
  mouse_write(0xe8);  /* Set wheel resolution */
  mouse_read();  /* ack */
  mouse_write(0x03);  /* 8 counts per mm */
  mouse_read();  /* ack */
  mouse_write(0xe6);  /* scaling 1:1 */
  mouse_read();  /* ack */
  mouse_write(0xf3);  /* Set sample rate */
  mouse_read();  /* ack */
  mouse_write(0x28);  /* Set sample rate */
  mouse_read();  /* ack */
  mouse_write(0xf4);  /* Enable device */
  mouse_read();  /* ack */  
  mouse_write(0xf0);  /* remote mode */
  mouse_read();  /* ack */    
  delayMicroseconds(100);  
  return true;
}


bool PS2_class::IsResponding()
{
 /* Function will return true if a mouse is connected and is responding */
 gohi(_ps2data);
 gohi(_ps2clk);
 delayMicroseconds(300);
 golo(_ps2clk);
 delayMicroseconds(300);
 golo(_ps2data);
 delayMicroseconds(10);
 gohi(_ps2clk);
 for(uint8_t i = 0; i < 200; i++) {
     if(digitalRead(_ps2clk) == LOW){    /* wait for mouse to take control of clock); */
        gohi(_ps2clk);
        delay(50);  // wait for mouse to time out
        mouse_available=true;
        return true;   // The mouse has responded          
     }
     delay(1);
 } 
 return false;  // mouse did not grab clock
}


PS2_class::PS2_class(int clk, int data)
{
  _ps2clk = clk;
  _ps2data = data;
//  memset( &mbuts,0, sizeof(mbuts));
for (int i=0;i<2;i++)
   mbuts[i] = 0; 

}


void PS2_class::mouse_get(signed char &_mx, signed char &_my,signed char &_mz, int &_mbut)
{ 
  if (!mouse_available) return;
  /* get a reading from the mouse */    
  mouse_write(0xeb);  /* give me data! */
  mouse_read();      /* ignore ack */
  mstat = mouse_read();
  mx = _mx = mouse_read();
  my = _my = mouse_read();
  mz = _mz = mouse_read();
  _mbut= mstat & 7;    // filter out 3 mouse buttons
}


// returns 0 = no press, 1 = key pressed, 2 = key released, 255 if key is down
int  PS2_class::mouse_getclick(int but)
{
  if(but>2) return 0;
  int retval=255;

  if (mstat & (1<<but))
  {
    if (mbuts[but] == 0)   retval=1;
  }
  else
    if (mbuts[but] == 255) retval=2;
       else retval=0;

  mbuts[but] = retval;
  return retval;
}