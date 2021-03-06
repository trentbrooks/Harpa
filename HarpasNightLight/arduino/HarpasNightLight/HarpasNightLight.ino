

// comment/uncomment these to test memory footprints
//#define USE_SERIAL_PRINT
#define USE_ETHERNET
#define USE_BONJOUR // cannot have bonjour without ethernet
#define USE_OSC

#ifdef USE_ETHERNET
#include <Ethernet.h> // http://arduino.cc/en/reference/ethernet
#include <SPI.h>
#ifdef USE_BONJOUR
#include <EthernetBonjour.h> // https://github.com/neophob/EthernetBonjour
#endif
#endif
#ifdef USE_OSC
#include <ArdOSC.h> // https://github.com/recotana/ArdOSC
#endif
#include "ColorUtils.h" // custom
#include <Timer.h> // https://github.com/JChristensen/Timer
#include <Adafruit_NeoPixel.h>


// -------
// POWER
//boolean isOn = true;


// TIMER
Timer timer; // used to replace standard loop
int8_t normalTimerId=-1;
int8_t speakerTimerId=-1;
int8_t switchOffTimerId=-1;
unsigned long killAfterHours =  43200000;//1000*60*60*12; 28800000;//1000*60*60*8;
unsigned long sleepAfterHours = 43200000;//1000*60*60*12; 57600000;//1000*60*60*16;

// MICROPHONE: reads mic level, compares against average. once loud enough sounds
// detected - it waits a certain amount (eg. 15 seconds) of time before resetting.
const int MIC_PIN = A1;   // the SPL output is connected to analog pin 0
boolean useMic = false;
const int MAX_MIC_SAMPLES = 120;//120;
int micReadCount = 15; // max number of samples for average
int micValues[MAX_MIC_SAMPLES]; // same number of samples
float differenceScale = 0.15;//0.5; // how different the read needs to be from the average (0.0 = same, 1.0 = opposite/very different)
int micSum = 0; // sound levels total
int micPos = 0; // array index
int soundResetCount = 0;
int soundResetTimeLimit = 5;//30;
boolean soundChanged = false;
boolean fadeOnSound = false;
int soundLevel = 0; // mic level
float soundRange = 700.0f; // mic sensor range: 0 (min) - 700 (max)


// SPEAKER: plays white noise
const int SPEAKER_PIN = 8;
boolean useSpeaker = false;
float noiseFrequencyDelay = 0.05; // used to be 50
unsigned long int reg = 0x55aa55aaL;


// RGB LEDS: fades/lerps between colours.
// if mode = 0 (eg. every 5 seconds), or can be changed manually via osc (1) or sound detection (2).
const int NUM_LEDS = 30;
const int LED_PIN = 7;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
Color currentLedRGB[NUM_LEDS];
Color newLedRGB[NUM_LEDS];
int hues[NUM_LEDS];
int latestHue = 22;
int brightness = 75; //255;
int saturation = 0;
int activeLeds = NUM_LEDS;
float lerpInc = 0.025;//0025;//025;//5;
float lerpAmount = 0;
int fadeDelayTime= 25000;//5000;
long previousMillis = 0;
uint16_t rainbowCycleIndex = 0;
int mode = 0; // 0 = normal/manual, 1 = auto fade/change, 2 = rainbow, 3 = sound reactive/mic

// NETWORK/OSC: some settings can be changed via osc (eg. from iphone). make sure correct ip is used on phone if no bonjour.
int ethernetConnected = 0;
boolean hasEthernetBlinked = false;
#ifdef USE_OSC
const int OSC_PORT = 5556;
OSCServer oscServer;
#endif
#ifdef USE_BONJOUR
const int BONJOUR_PORT = 7777;
const char BONJOUR_SERVICE[] = "Arduino._ofxBonjourIp";
#endif


// ----------------------------------------------------------------------
void setup() {

  // baud rate
  Serial.begin(38400); //19200

  //Serial.println("Memory: Setup begin=");
  //Serial.println(freeMemory());

  // Initialize all pixels/LEDS to white
  strip.begin();
  int hue = 22; //orange
  latestHue = hue;
  Color clr;
  ColorUtils::setHSB(clr, hue, saturation, brightness);
  for(int i = 0; i < activeLeds; i++)  {
    hues[i] = hue;
    strip.setPixelColor(i, clr.r, clr.g, clr.b);
  }
  strip.show(); // Initialize all pixels to 'off'
  
  
  // network
#ifdef USE_ETHERNET
  byte myMac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  ethernetConnected = Ethernet.begin(myMac); // using a default myMac address, can also pass ip + mac address: (myMac ,myIp);
  // print arduino local IP address:
#ifdef USE_SERIAL_PRINT
  Serial.print("\nEthernet connected: ");
  Serial.print(ethernetConnected);
  Serial.print("\nArduino IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
#endif

  // bonjour: easy to discover arduino when service is registered on local network (don't need to know ip address)
  // device is "Arduino", service is "_ofxBonjourIp._tcp"
#ifdef USE_BONJOUR
  int bonjourConnected = EthernetBonjour.begin(); // entering a name here has no effect
#ifdef USE_SERIAL_PRINT
  Serial.print("\nBonjour connected: ");
  Serial.print(bonjourConnected);
#endif
  EthernetBonjour.addServiceRecord(BONJOUR_SERVICE, BONJOUR_PORT, MDNSServiceTCP);
#endif
#endif

  // osc
#ifdef USE_OSC
  int oscConnected = oscServer.begin(OSC_PORT); //ardosc
#ifdef USE_SERIAL_PRINT
  Serial.print("\nOSC connected: ");
  Serial.print(oscConnected);
  Serial.print(" Port: ");
  Serial.print(OSC_PORT);
#endif

  // set callback functions for osc
  oscServer.addCallback("/hue",&onHue);
  oscServer.addCallback("/brightness",&onBrightness);
  oscServer.addCallback("/saturation",&onSaturation);
  oscServer.addCallback("/numleds",&onLedCount); // NEW
  oscServer.addCallback("/leds",&onLeds); // NEW
  oscServer.addCallback("/fadedelay",&onFadeDelay); // NEW  
  oscServer.addCallback("/mode",&onMode);
  oscServer.addCallback("/microphone",&onMicrophone);
  oscServer.addCallback("/micsamples", &onMicSamples); // NEW
  oscServer.addCallback("/micdiff", &onMicDifferenceScale); // NEW
  oscServer.addCallback("/speaker",&onSpeaker);
  oscServer.addCallback("/noisefrequency",&onNoiseFrequency);

#endif

  // inputs + outputs
  pinMode(MIC_PIN, INPUT); // mic pin
  pinMode(SPEAKER_PIN, OUTPUT); // speaker pin

  

  // if the internet is connected all leds start green, otherwise red
  /*for(int i = 0; i < NUM_LEDS; i++)  {
      if(ethernetConnected == 1) {
        // all lights are green - we are connected
        hues[i] = 85;
        ColorUtils::setHSB(newLedRGB[i], hues[i], saturation, brightness);
      } else {
        // all lights are red
        hues[i] = 0;
        ColorUtils::setHSB(newLedRGB[i], hues[i], saturation, brightness);
      }
  }*/

  // timers
  normalTimerId = timer.every(20, onTimerUpdate, 0); // normal update loop
  speakerTimerId = timer.every(noiseFrequencyDelay, onTimerSpeaker, 0); // speaker needs seperate loop
  startOffTimer(); 
  //switchOffTimerId = timer.after(killAfterHours, switchLedsOff, 0); // kill everything after X hours
  
  //Serial.println(killAfterHours);
  //Serial.println("Memory: Setup complete=");
  //Serial.println(freeMemory());
#ifdef USE_SERIAL_PRINT
  Serial.println("\nSetup complete");
#endif
}

void loop() {

  // all the leds initialise green when connected to netwrok or red when not
  if(!hasEthernetBlinked) {
    
    Color clr;
    saturation = 255; // reset saturation
    if(ethernetConnected == 1) {

      // all lights are green - we are connected
      //Color green = {0,255,0};
      int hue = 85; //green
      ColorUtils::setHSB(clr, hue, saturation, brightness);
      //colorWipe(clr, 50);
      //theaterChase(clr, 50);

      /*lerpAmount = 0.0;
      for(int i = 0; i < NUM_LEDS; i++)  {
        hues[i] = 85;
        ColorUtils::setHSB(newLedRGB[i], hues[i], saturation, brightness);
      }*/
    } else {

      // all lights are red
      //Color red = {255,0,0};
      int hue = 0; //red
      ColorUtils::setHSB(clr, hue, saturation, brightness);
      

      /*lerpAmount = 0.0;
      for(int i = 0; i < NUM_LEDS; i++)  {
        hues[i] = 0;
        ColorUtils::setHSB(newLedRGB[i], hues[i], saturation, brightness);
      }*/
    }
    
    //colorWipe(clr, 50);
    theaterChase(clr, 50);
    
    // after inited - all leds go to default mode and go to a single colour
    int hue = 22; // orange
    latestHue = hue;
    for(int i = 0; i < activeLeds; i++)  {
      hues[i] = hue;
      ColorUtils::setHSB(newLedRGB[i], hues[i], saturation, brightness);
    }
    //Serial.print("\nShould be orange now");
    //strip.show(); 
    
    hasEthernetBlinked = true;
  }

  // bonjour + osc run all the time
#ifdef USE_ETHERNET
#ifdef USE_BONJOUR
  EthernetBonjour.run();
#endif
#endif
#ifdef USE_OSC
  oscServer.aviableCheck(); // > 0
#endif

  // if not powered don't do anything else
  // TODO: make sure everything is turned off first
  //if(!isOn) return;

  // using timers instead of normal loop for...
  // 1. updateLeds()
  // 2. updateMic()
  // 3. updateSpeaker() (custom timer)
  //Serial.print("\nDa fuq?");
  timer.update();
}

void onTimerUpdate(void *context) {

  // 0 = normal/manual, 1 = auto fade/change, 2 = rainbow, 3 = sound reactive/mic
  if(mode == 0) {
    // manually change lights via osc
    updateLeds();
  }
  else if(mode == 1) {
    // automatic mode change colours based on a timer
    autoRandomiseColor();
    updateLeds();
  }
  else if(mode == 2) {
    // rainbow cycle
    rainbowCycleColor();
  }
  else if(mode == 3) {
    // change lights based on sound input
    updateMic();
    updateLeds();
  }

}

// ------- MIC
void updateMic() {

  if(useMic) {

    // detect sound level, rolling average, and difference
    soundLevel = analogRead(MIC_PIN);
    int avg = getRollingAverage(soundLevel);
    float diff = abs(soundLevel - avg) / soundRange; // normalised 0 - 1.0f

    // 1) sound either turns leds on/off (defined by fadeOnSound), then waits for delay before resetting
    // eg. if baby cries, then the lights go off - this would be a cruel way to train a baby not to cry.
    /*if(!soundChanged) {
     // check if current read is x amount greater than average
     if(diff > differenceScale)  {
     soundChanged = true;
     brightness = (fadeOnSound) ? 0 : 255;
     setHSB(newRGB, hue, 255, brightness);
     }
     }
     else {
     // reset after threshold
     soundResetCount++;
     if(soundResetCount > soundResetTimeLimit) {
     soundChanged = false;
     }
     }*/

    // 2) sound could change the colour of the leds instead of turning led on/off
    //if(!soundChanged) {
      if(diff > differenceScale) {
        /*#ifdef USE_SERIAL_PRINT
        Serial.print("\nSound changed: ");
        Serial.print(diff);
        #endif*/
        soundChanged = true;
        lerpAmount = 0;
        int randomHue = random(0,255);
        for(int i = 0; i < activeLeds; i++)  {
          hues[i] = randomHue;
          ColorUtils::setHSB(newLedRGB[i], hues[i], saturation, brightness);
        }
      }
    //}
    /*else {
      // reset after threshold
      soundResetCount++;
      if(soundResetCount > soundResetTimeLimit) {
        soundChanged = false;
        soundResetCount = 0;
        //Serial.println("  time reset");
      }
    }*/
  }
}

// eg. micReadCount = 120 (number of samples collected)
int getRollingAverage(int v) {
  micSum -= micValues[micPos];  // only need the array to subtract old value
  micSum += v;
  micValues[micPos] = v;
  //micPos = (micPos + 1) % micReadCount;
  if (++micPos >= micReadCount) micPos = 0;
  return micSum / micReadCount;
}


// ------- SPEAKER
void onTimerSpeaker(void *context) {
  //Serial.println("got speaker?");
  updateSpeaker();
}

void updateSpeaker() {
  if(useSpeaker) generateNoise();
}

void generateNoise(){

  unsigned long int newr;
  unsigned char lobit;
  unsigned char b31, b29, b25, b24;
  b31 = (reg & (1L << 31)) >> 31;
  b29 = (reg & (1L << 29)) >> 29;
  b25 = (reg & (1L << 25)) >> 25;
  b24 = (reg & (1L << 24)) >> 24;
  lobit = b31 ^ b29 ^ b25 ^ b24;
  newr = (reg << 1) | lobit;
  reg = newr;
  digitalWrite (SPEAKER_PIN, reg & 1);

  // use timer instead of below
  //delayMicroseconds (noiseFrequencyDelay); // Changing this value changes the frequency.
}


// ------- RGB LEDS
// update individual leds
void updateLeds() {

  if(lerpAmount < 1) {
    lerpAmount += lerpInc;
  } else {
    lerpAmount = 1;
  }

  for(int i = 0; i < NUM_LEDS; i++)  {
    ColorUtils::lerpRGB(currentLedRGB[i], newLedRGB[i], lerpAmount);
    //uint32_t hexForLed =  ColorUtils::rgbToHex(currentLedRGB[i].r,currentLedRGB[i].g,currentLedRGB[i].b);
    strip.setPixelColor(i, currentLedRGB[i].r,currentLedRGB[i].g,currentLedRGB[i].b);
  }
  strip.show();
  
  /*#ifdef USE_SERIAL_PRINT
  Serial.print("\nLEDs updated: ");
  Serial.print(lerpAmount);
  #endif*/

}

void autoRandomiseColor() {
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > fadeDelayTime) {
    previousMillis = currentMillis;
    lerpAmount = 0;
    //ColorUtils::setHSB(newRGB, hue, saturation, brightness);
    int randomHue = random(0,255);
    for(int i = 0; i < activeLeds; i++)  {
      hues[i] = randomHue;
      ColorUtils::setHSB(newLedRGB[i], hues[i], saturation, brightness);
    }
  }
}



void rainbowCycleColor() {
  uint16_t i;
  //Color c;

  for(i=0; i<NUM_LEDS; i++) {
      
      //strip.setPixelColor(i, Wheel((i+j) & 255));
      //i * 256 / strip.numPixels()
      //wheel(c, (i+rainbowIndex) & 255); // wrong!
      int hue = ((i * 256) / NUM_LEDS +rainbowCycleIndex) & 255;
      //int hue = (i+rainbowCycleIndex) & 255;
      //wheel(c, ((i * 256) / NUM_LEDS +rainbowIndex) & 255);
      //ColorUtils::setRGB(newLedRGB[i], c.r, c.g, c.b);
      //ColorUtils::setRGB(currentLedRGB[i], c.r, c.g, c.b);
      ColorUtils::setHSB(newLedRGB[i], hue, saturation, brightness);
      strip.setPixelColor(i, newLedRGB[i].r, newLedRGB[i].g, newLedRGB[i].b);
      //strip.setPixelColor(i, Wheel((i+j) & 255));
  }
  strip.show();
    
  if (++rainbowCycleIndex == 255) rainbowCycleIndex = 0;
}

// Adafruit example animations (modified to use Color struct)
// note - these rely on delays
// for compatibility with own color tweeening library, also set my arrays to same as adafruit lib
// Fill the dots one after the other with a color
void colorWipe(Color& c, uint8_t wait) {

  // no need to lerp colors, due to delays
  lerpAmount = 1.0;

  for(int i=0; i<strip.numPixels(); i++) {

      ColorUtils::setRGB(newLedRGB[i], c.r, c.g, c.b);
      ColorUtils::setRGB(currentLedRGB[i], c.r, c.g, c.b);
      strip.setPixelColor(i, c.r, c.g, c.b);
      strip.show();
      delay(wait);
  }
}

// Theatre-style crawling lights.
void theaterChase(Color& c, uint8_t wait) {

  // no need to lerp colors, due to delays
  lerpAmount = 1.0;

  for (int j=0; j<30; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        ColorUtils::setRGB(newLedRGB[i+q], c.r, c.g, c.b);
        ColorUtils::setRGB(currentLedRGB[i+q], c.r, c.g, c.b);
        strip.setPixelColor(i+q, c.r, c.g, c.b);    //turn every third pixel on
      }
      strip.show();

      delay(wait);
      for (int i=0; i < strip.numPixels(); i=i+3) {
        ColorUtils::setRGB(newLedRGB[i+q], 0,0,0);
        ColorUtils::setRGB(currentLedRGB[i+q], 0,0,0);
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// copying adafruit rainbow stuff to use Color struct
void rainbow(uint8_t wait) {
  uint16_t i, j;
  Color c;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      
      //strip.setPixelColor(i, Wheel((i+j) & 255));
      wheel(c, i); // wrong!
      ColorUtils::setRGB(newLedRGB[i], c.r, c.g, c.b);
      ColorUtils::setRGB(currentLedRGB[i], c.r, c.g, c.b);
      strip.setPixelColor(i, c.r, c.g, c.b);
      //strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
/*void rainbowCycle(uint8_t wait) {
  uint16_t i, j;
  //Color c;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}*/

void wheel(Color& clr, byte WheelPos) {

  if(WheelPos < 85) {
   ColorUtils::setRGB(clr, WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   ColorUtils::setRGB(clr, 255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   ColorUtils::setRGB(clr, 0, WheelPos * 3, 255 - WheelPos * 3);
  }
}



// ------- OSC events
// ifdef not working here
#ifdef USE_OSC
void onHue(OSCMessage *_mes){
  int oscVal = _mes->getArgInt32(0);
  if(oscVal == latestHue) return;
  latestHue = oscVal;
  //ColorUtils::setHSB(newRGB, hue, saturation, brightness);
  for(int i = 0; i < activeLeds; i++)  {
    hues[i] = latestHue;
    ColorUtils::setHSB(newLedRGB[i], hues[i], saturation, brightness);
  }
#ifdef USE_SERIAL_PRINT
  Serial.print("\nHues changed: ");
  Serial.print(latestHue);
#endif
}


void onBrightness(OSCMessage *_mes){
  int oscVal = _mes->getArgInt32(0);
  if(oscVal == brightness) return;
  brightness = oscVal;
  //ColorUtils::setHSB(newRGB, hue, saturation, brightness);
  for(int i = 0; i < activeLeds; i++)  {
    ColorUtils::setHSB(newLedRGB[i], hues[i], saturation, brightness);
  }
#ifdef USE_SERIAL_PRINT
  Serial.print("\nBrightness changed: ");
  Serial.print(brightness);
#endif

  // reset the timers
  if(brightness == 0) {
    // when we send 'black' (ie. off) reset the ON timer
    startOnTimer();
  } else {
    // when we send and other value reset the OFF timer
    startOffTimer();
  }
}

void onSaturation(OSCMessage *_mes){
  int oscVal = _mes->getArgInt32(0);
  if(oscVal == saturation) return;
  saturation = oscVal;
  //ColorUtils::setHSB(newRGB, hue, saturation, brightness);
  for(int i = 0; i < activeLeds; i++)  {
    ColorUtils::setHSB(newLedRGB[i], hues[i], saturation, brightness);
  }
#ifdef USE_SERIAL_PRINT
  Serial.print("\nSaturation changed: ");
  Serial.print(saturation);
#endif
}

void onLedCount(OSCMessage *_mes) {
  activeLeds = _mes->getArgInt32(0);
  
  // switch the other led's to black/off
  for(int i = 0; i < NUM_LEDS; i++)  {
    if(i > activeLeds) {
      ColorUtils::setHSB(newLedRGB[i], 0, 0, 0);
      ColorUtils::setHSB(currentLedRGB[i], 0, 0, 0);
    }  
  }
#ifdef USE_SERIAL_PRINT
  Serial.print("\nActive leds changed: ");
  Serial.print(activeLeds);
#endif
}

void onFadeDelay(OSCMessage *_mes) {
  fadeDelayTime = _mes->getArgInt32(0);
#ifdef USE_SERIAL_PRINT
  Serial.print("\nFade delay changed: ");
  Serial.print(fadeDelayTime);
#endif
}
  

void onMode(OSCMessage *_mes){
  int oscVal = _mes->getArgInt32(0);
  if(oscVal == mode) return;
  mode = oscVal;
  if(mode == 3) {
    useMic = true;
    int hue = 220; // everything goes pink - fade mode
    //ColorUtils::setHSB(newRGB, hue, saturation, brightness);
    for(int i = 0; i < activeLeds; i++)  {
      hues[i] = hue;
      ColorUtils::setHSB(newLedRGB[i], hues[i], saturation, brightness);
    }
  } else {
    useMic = false;
  }
#ifdef USE_SERIAL_PRINT
  Serial.print("\nMode changed: ");
  Serial.print(mode);
#endif
}

void onMicrophone(OSCMessage *_mes){
  useMic = _mes->getArgInt32(0);
#ifdef USE_SERIAL_PRINT
  Serial.print("\nMicrophone changed: ");
  Serial.print(useMic);
#endif
}

void onMicSamples(OSCMessage *_mes) {
  micPos = 0;
  micReadCount = _mes->getArgInt32(0); // 0-120
  micSum = 0;
  micValues[micPos] = 0;
  if(micReadCount > MAX_MIC_SAMPLES) micReadCount = MAX_MIC_SAMPLES;
#ifdef USE_SERIAL_PRINT
  Serial.print("\nMicrophone samples changed: ");
  Serial.print(micReadCount);
#endif
}

void onMicDifferenceScale(OSCMessage *_mes) {
  differenceScale = _mes->getArgFloat(0); // 0-1.0
#ifdef USE_SERIAL_PRINT
  Serial.print("\nMicrophone diff scale changed: ");
  Serial.print(differenceScale);
#endif
}

void onSpeaker(OSCMessage *_mes){
  useSpeaker = _mes->getArgInt32(0);
#ifdef USE_SERIAL_PRINT
  Serial.print("\nSpeaker changed: ");
  Serial.print(useSpeaker);
#endif
}


void onNoiseFrequency(OSCMessage *_mes){
  noiseFrequencyDelay = _mes->getArgFloat(0); // 0-0.2
  timer.stop(speakerTimerId);
  speakerTimerId = timer.every(noiseFrequencyDelay, onTimerSpeaker, 0); // speaker needs seperate loop
#ifdef USE_SERIAL_PRINT
  Serial.print("\nNoise frequency changed: ");
  Serial.print(noiseFrequencyDelay);
#endif
}

void onLeds(OSCMessage *_mes) {
  
  boolean ledsOn = (_mes->getArgInt32(0) == 1) ? true : false;
  for(int i = 0; i < NUM_LEDS; i++)  {
    if(ledsOn) {
      saturation = 255;
      brightness = 255; // should check if this is too bright
      ColorUtils::setHSB(newLedRGB[i], hues[i], saturation, brightness);
      ColorUtils::setHSB(currentLedRGB[i], hues[i], saturation, brightness);
    } else {
      saturation = 255;
      brightness = 0;
      ColorUtils::setHSB(newLedRGB[i], hues[i], saturation, brightness);
      ColorUtils::setHSB(currentLedRGB[i], hues[i], saturation, brightness);
    }
  }
  #ifdef USE_SERIAL_PRINT
  Serial.print("\nLEDS changed: ");
  Serial.print(ledsOn);
#endif
}
  
// TODO: this isn't really power - when turning back 'on' need to go through the ethernet startup cycle again
/*void onPower(OSCMessage *_mes){
  isOn = (_mes->getArgInt32(0) == 1) ? true : false;
  for(int i = 0; i < NUM_LEDS; i++)  {
    if(!isOn) {
      // switch all lights off- to black
      saturation = 0;
      brightness = 0;
      hues[i] = 0;
      ColorUtils::setHSB(newLedRGB[i], hues[i], saturation, brightness);
      ColorUtils::setHSB(currentLedRGB[i], hues[i], saturation, brightness);
    }
    else {
      // switch all lights on- to white
      hues[i] = 255;
      saturation = 255;
      brightness = 255;
      ColorUtils::setHSB(newLedRGB[i], hues[i], saturation, brightness);
    }

  }
#ifdef USE_SERIAL_PRINT
  Serial.print("\nPower changed: ");
  Serial.print(isOn);
#endif
}*/

#endif

// POWER OFF
void switchLedsOff(void *context) {
  
  //Serial.println("Finished!");
  
  // 1. switch all lights off- to black
  saturation = 0;
  brightness = 0;
  for(int i = 0; i < NUM_LEDS; i++)  {
      hues[i] = 0;
      ColorUtils::setHSB(newLedRGB[i], hues[i], saturation, brightness);
      ColorUtils::setHSB(currentLedRGB[i], hues[i], saturation, brightness);
  }
  
  // 4. start kill timer again
  startOffTimer();
  
  // 2. stay off for the next X hours
  /*delay(sleepAfterHours); 
  
  // 3. turn LEDS back on to default orange
  brightness = 75; //255;
  saturation = 0;
  int hue = 22; // orange
  for(int i = 0; i < NUM_LEDS; i++)  {
      hues[i] = hue;
      ColorUtils::setHSB(newLedRGB[i], hues[i], saturation, brightness);
  }
  
  // 4. start kill timer again
  switchOffTimerId = timer.after(killAfterHours, switchLedsOff, 0); // kill everything after X hours
  */
}

// 1. this turns the LEDS OFF after X hours. This should launch on startup.
void startOffTimer() {
  timer.stop(switchOffTimerId);
  switchOffTimerId = timer.after(killAfterHours, switchLedsOff, 0);
}

// 2. waits for X hours then turns the LEDS ON. Assumes everything is already off.
void startOnTimer() {
  timer.stop(switchOffTimerId);
  switchOffTimerId = timer.after(sleepAfterHours, switchLedsOn, 0); // kill everything after X hours
}




// POWER ON
void switchLedsOn(void *context) {
  brightness = 75; //255;
  saturation = 0;
  int hue = 22; // orange
  for(int i = 0; i < NUM_LEDS; i++)  {
      hues[i] = hue;
      ColorUtils::setHSB(newLedRGB[i], hues[i], saturation, brightness);
  }
  
  startOnTimer();
  
}

