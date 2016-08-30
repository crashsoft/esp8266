#pragma once
#include "Arduino.h"

#define MBUT_NOPRESS	0
#define MBUT_PRESS	1
#define MBUT_RELEASE 	2
#define MBUT_DOWN 	255


class PS2_class {
public:
  PS2_class(int clk, int data);
  bool mouse_init();
  void mouse_get(signed char &mx, signed char &my, signed char &mz, int &mbut);
  int  mouse_getclick(int but);
  bool isMouse() {return mouse_available;}
 
private:
   signed int  mx, my, mz;
   int	mstat;
   int	mbuts[3];
   bool mouse_available=0;
   int _ps2clk;
   int _ps2data;
   void golo(const int &pin);
   void gohi(const int &pin); 
   void mouse_write(char data);
   char mouse_read(void);
   bool IsResponding();    
};