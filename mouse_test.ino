// pins on mouse from left to right: MCLK, MDATA, +5V, GND

#include <pgmspace.h>
#include "ssd1306_i2c.h"

#define MDATA 12
#define MCLK 13
#define SDA_PIN 5
#define SCL_PIN 4
#define countof(a) (sizeof(a) / sizeof(a[0]))

struct MouseClass
{ 
  public:
  int   x, y, z;
  int   but;
};

int           mouse_available=0;
char          strbuf[80];                      // common buffer
int           curX=64, curY=30;
bool          display_invert=0;

SSD1306 display(0x3c, SDA_PIN, SCL_PIN);

void setup()
{
  Serial.begin(115200);  
  Serial.print("Mouse: ");
  mouse_available = mouse_init(); 
  Serial.println(mouse_available);  
    
  display.init();  
  display.flipScreenVertically();
  display.clear();
  display.display();
}

/*
 * get a reading from the mouse and report it back to the
 * host via the serial line.
 */
void loop()
{
  char mstat;
  signed char mx, my,mz;
  int mbut;
  
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  
  if (mouse_available) {
  /* get a reading from the mouse */
  mouse_write(0xeb);  /* give me data! */
  mouse_read();      /* ignore ack */
  mstat = mouse_read();
  mx = mouse_read();
  my = mouse_read();
  mz = mouse_read();
  mbut= mstat & 7;    // filter out 3 mouse buttons
  if (mbut & 4) {
    display_invert=!display_invert;
    display.invertDisplay(display_invert);
  }

  curX +=mx; curY -=my;
  curX = constrain(curX,0,127);
  curY = constrain(curY,0,63);
  
  snprintf(strbuf, countof(strbuf), "x: %03i", mx);      
  display.drawString(0, 0, strbuf);
  snprintf(strbuf, countof(strbuf), "y: %03i", my);
  display.drawString(40, 0, strbuf);
  snprintf(strbuf, countof(strbuf), "z: %03i",mz);      
  display.drawString(80, 0, strbuf);
  snprintf(strbuf, countof(strbuf), "but: %02u",mbut);      
  display.drawString(0,10, strbuf);
  }
  else Serial.println("No mouse:(");

  //display.drawString(0, 0, "Huh huh");
  display.setFont(ArialMT_Plain_16);  
  display.drawString(0, 42, "Testing mouse");  
  display.setPixel(curX, curY);
  
  display.display();
  delay(100);  /* twiddle */
  
}


void gohi(int pin)
{
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH);
}

void golo(int pin)
{
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void mouse_write(char data)
{
  char i;
  char parity = 1;  
  /* put pins in output mode */
  gohi(MDATA);
  gohi(MCLK);
  delayMicroseconds(300);
  golo(MCLK);
  delayMicroseconds(300);
  golo(MDATA);
  delayMicroseconds(10);
  /* start bit */
  gohi(MCLK);
  
  /* wait for mouse to take control of clock); */
  while (digitalRead(MCLK) == HIGH);
    
  /* clock is low, and we are clear to send data */
  for (i=0; i < 8; i++) {
    if (data & 0x01) 
          gohi(MDATA);    
    else  golo(MDATA);
    
    /* wait for clock cycle */
    while (digitalRead(MCLK) == LOW);
    while (digitalRead(MCLK) == HIGH);
    parity = parity ^ (data & 0x01);
    data = data >> 1;
  }  
  /* parity */
  if (parity)
         gohi(MDATA);  
  else   golo(MDATA);
    
  while (digitalRead(MCLK) == LOW);
  while (digitalRead(MCLK) == HIGH);
  /* stop bit */
  gohi(MDATA);
  delayMicroseconds(50);
  while (digitalRead(MCLK) == HIGH);
  /* wait for mouse to switch modes */
  while ((digitalRead(MCLK) == LOW) || (digitalRead(MDATA) == LOW));
  /* put a hold on the incoming data. */
  golo(MCLK);    
}

/*
 * Get a byte of data from the mouse
 */
char mouse_read(void)
{
  int i;
  char data = 0x00;  
  char bit = 0x01;
  
  /* start the clock */
  gohi(MCLK);
  gohi(MDATA);
  delayMicroseconds(50);
  while (digitalRead(MCLK) == HIGH);
  delayMicroseconds(5);  /* not sure why */
  while (digitalRead(MCLK) == LOW); /* eat start bit */
    
  for (i=0; i < 8; i++) {
    while (digitalRead(MCLK) == HIGH);
    if (digitalRead(MDATA) == HIGH) {
      data = data | bit;
    }
    while (digitalRead(MCLK) == LOW);
    bit = bit << 1;
  }
  /* eat parity bit, which we ignore */
  while (digitalRead(MCLK) == HIGH);
  while (digitalRead(MCLK) == LOW);
  /* eat stop bit */
  while (digitalRead(MCLK) == HIGH);
  while (digitalRead(MCLK) == LOW);
  /* put a hold on the incoming data. */
  golo(MCLK);
  return data;
}

bool mouse_init(){
  char mouseId;
  
  if (!IsResponding())
     return false;
   
  gohi(MCLK);
  gohi(MDATA);
  
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



boolean IsResponding()
{
 /* Function will return true if a mouse is connected and is responding */
 gohi(MDATA);
 gohi(MCLK);
 delayMicroseconds(300);
 golo(MCLK);
 delayMicroseconds(300);
 golo(MDATA);
 delayMicroseconds(10);
 gohi(MCLK);
 for(uint8_t i = 0; i < 200; i++) {
     if(digitalRead(MCLK) == LOW){    /* wait for mouse to take control of clock); */
        gohi(MCLK);
        delay(50);  // wait for mouse to time out
        return true;   // The mouse has responded          
     }
     delay(1);
 }
 return false;  // mouse did not grab clock
}
