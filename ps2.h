#pragma once
#include "Arduino.h"

class PS2_class {
public:
  PS2_class(int clk, int data);
  bool mouse_init();
  void mouse_get(signed char &mx, signed char &my, signed char &mz, int &mbut);
  bool isMouse() {return mouse_available;}
 
private:
   bool mouse_available=0;
   int _ps2clk;
   int _ps2data;
   void golo(const int &pin);
   void gohi(const int &pin); 
   void mouse_write(char data);
   char mouse_read(void);
   bool IsResponding();    
};
