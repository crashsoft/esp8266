// pins on mouse from left to right: _ps2clk, _ps2data, +5V, GND

#include <pgmspace.h>
#include <ps2.h>
#include <brzo_i2c.h>
#include "SSD1306Brzo.h"

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

char          strbuf[80];                      // common buffer
int           curX=64, curY=30;
bool          display_invert=0;


PS2_class mouse(MCLK, MDATA);
SSD1306Brzo  display(0x3c, SDA_PIN, SCL_PIN);

void setup()
{
  Serial.begin(115200);  
  Serial.print("Mouse: ");  
  Serial.println(mouse.mouse_init()); 

  Serial.print("Init display..");
  display.init();  
  display.flipScreenVertically();
  display.clear();
  display.display();
  Serial.println("Ok!");
}

void loop()
{  
  signed char mx, my,mz;
  int mbut;
  
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
    
  if (mouse.isMouse()) {
    mouse.mouse_get(mx,my,mz,mbut);
    curX +=mx; curY -=my;
    curX = constrain(curX,0,127);
    curY = constrain(curY,0,63);

    if (mbut & 4) {      
      display_invert=!display_invert;
      if (display_invert)
         display.invertDisplay();
      else display.normalDisplay();
    }

    if (mbut & 1) {      
       display.setContrast(curX *2);
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
  else display.drawString(0, 0, "No mouse :(");  

  //display.drawString(0, 0, "Huh huh");
  display.setFont(ArialMT_Plain_16);  
  display.drawString(0, 42, "Testing mouse");  
  display.setColor(INVERSE);
  display.setPixel(curX, curY);
  display.setColor(WHITE);
  
  display.display();
  delay(100);
}

