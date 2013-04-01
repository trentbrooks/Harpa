
#include "ColorUtils.h"

ColorUtils::ColorUtils(){
}



// modifies the passed colour by a 0-1 percentage to the to color.
void ColorUtils::lerpRGB(Color& clr, Color& to, float amount) {
  //Serial.println(amount);
  float invAmount = 1.0f - amount;
  clr.r = invAmount * clr.r + amount * to.r;
  clr.g = invAmount * clr.g + amount * to.g;
  clr.b = invAmount * clr.b + amount * to.b;
}

// http://www.pixelwit.com/blog/2007/05/hexadecimal-color-fading/
uint32_t ColorUtils::lerpHex (uint32_t hex, uint32_t hex2, float ratio){
  byte r = hex >> 16;
  byte g = hex >> 8 & 0xFF;
  byte b = hex & 0xFF;
  r += ((hex2 >> 16)-r)*ratio;
  g += ((hex2 >> 8 & 0xFF)-g)*ratio;
  b += ((hex2 & 0xFF)-b)*ratio;

  return (r<<16 | g<<8 | b);
}

// create a 24 bit color value from R,G,B
uint32_t ColorUtils::rgbToHex(byte r, byte g, byte b) {
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}


void ColorUtils::setHsb(Color& clr, float hue, float saturation, float brightness) {
  if(brightness == 0) { // black
    clr.r = clr.g = clr.b = 0;
  } 
  else if(saturation == 0) { // grays
    clr.r = clr.g = clr.b = brightness;
  } 
  else {
    float hueSix = hue * 6. / 255.;
    float saturationNorm = saturation / 255.;
    int hueSixCategory = (int) floor(hueSix);
    float hueSixRemainder = hueSix - hueSixCategory;
    unsigned char pv = (unsigned char) ((1.f - saturationNorm) * brightness);
    unsigned char qv = (unsigned char) ((1.f - saturationNorm * hueSixRemainder) * brightness);
    unsigned char tv = (unsigned char) ((1.f - saturationNorm * (1.f - hueSixRemainder)) * brightness);
    switch(hueSixCategory) {
    case 0: // r
      clr.r = brightness;
      clr.g = tv;
      clr.b = pv;
      break;
    case 1: // g
      clr.r = qv;
      clr.g = brightness;
      clr.b = pv;
      break;
    case 2:
      clr.r = pv;
      clr.g = brightness;
      clr.b = tv;
      break;
    case 3: // b
      clr.r = pv;
      clr.g = qv;
      clr.b = brightness;
      break;
    case 4:
      clr.r = tv;
      clr.g = pv;
      clr.b = brightness;
      break;
    case 5: // back to r
      clr.r = brightness;
      clr.g = pv;
      clr.b = qv;
      break;
    }
  }
}
