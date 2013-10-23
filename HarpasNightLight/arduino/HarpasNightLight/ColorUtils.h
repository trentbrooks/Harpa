
#ifndef COLOUR_UTILS
#define COLOUR_UTILS
#include "Arduino.h"

/*
  ColorUtils.
  - set of static helper classes for dealing with colors
  - most code is ripped from Openframeworks
*/

// -------
struct Color {
  byte r;
  byte g;
  byte b;
};

// -------
class ColorUtils
{
  public:
    ColorUtils();
    
    // static methods
    static uint32_t rgbToHex(byte r, byte g, byte b);
    static uint32_t lerpHex (uint32_t hex, uint32_t hex2, float ratio);
    static void lerpRGB(Color& from, Color& to, float amount);
    static void setHsb(Color& clr, float hue, float saturation, float brightness);
};


#endif
