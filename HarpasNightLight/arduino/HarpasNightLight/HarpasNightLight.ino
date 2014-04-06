
// comment/uncomment these to test memory footprints
#define USE_SERIAL_PRINT
#define USE_ETHERNET
#define USE_BONJOUR // cannot have bonjour without ethernet
#define USE_OSC
#define USE_NEOPIXELS

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
//#include <MemoryFree.h>
#ifdef USE_NEOPIXELS
#include <Adafruit_NeoPixel.h>
#endif


// MEMORY TEST
/*
http://arduino.cc/en/Tutorial/Memory
 http://playground.arduino.cc/Code/AvailableMemory
 Need to compare mega + uno
 arduino mega total = 7465 bytes (SRAM?)
 begin setup = 5933
 end of setup = 5747
 loop = 5761
 // max bytes for app..
 mega- 258048
 uno- 32256
 
 everything-34116
 no ethernet, bonjour, osc- 10600
 no bonjour-23420
 no osc-28930
 
 osc callbacks around -3000 bytes
 */

// -------
// TIMER
Timer timer; // used to replace standard loop

// POWER
boolean isOn = true;

// MICROPHONE: reads mic level, compares against average. once loud enough sounds
// detected - it waits a certain amount (eg. 15 seconds) of time before resetting.
int SPL_PIN = A1;   // the SPL output is connected to analog pin 0
int micReadCount = 120; // max number of samples for average
int micValues[120]; // same number of samples
float differenceScale = 0.5; // how different the read needs to be from the average (0.0 = same, 1.0 = opposite/very different)
int micSum = 0; // sound levels total
int micPos = 0; // array index
int soundResetCount = 0;
int soundResetTimeLimit = 30; 
boolean soundChanged = false;
boolean fadeOnSound = false;
boolean useMic = false;
int soundLevel = 0; // mic level
float soundRange = 700.0f; // mic sensor range: 0 (min) - 700 (max)

// SPEAKER: plays white noise or ?
int SPEAKER_PIN = 9;
float noiseFrequencyDelay = .05; // used to be 50
int8_t noiseTimerId;
boolean useSpeaker = true;

/*
fucked the leds... need to buy more
 light 1 has no blue,
 light 2,3 ok
 light 4 has no blue or green
 */

// RGB LEDS: fades/lerps between colours. led's are daisy chained. they change automatically
// if mode = 0 (eg. every 5 seconds), or can be changed manually via osc (1) or sound detection (2).
#ifdef USE_NEOPIXELS
int numNeoPixels = 4;
int  neoPin = 5;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numNeoPixels, neoPin, NEO_GRB + NEO_KHZ800);
#endif
int LED_CLOCK_PIN = 2; // clock pin
int LED_SERIAL_PIN = 3; // serial data pin
uint32_t currentHex = 0xff0000;
uint32_t newHex = 0xffff00;
//Color currentRGB = {0,0,255}; // from color
//Color newRGB = {255,255,0}; // to color
int lightCount = 4; // Number of RGBLED modules connected
Color currentLedRGB[] = { {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0} }; 
//Color newLedRGB[] = { {0,255,0}, {0,255,0}, {255,127,0}, {255,255,0}, {255,0,0} };
Color newLedRGB[] = { {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0} };
int hues[] = { 0,0,0,0}; // custom per led
int brightness = 255;
int saturation = 255;
float lerpInc = 0.0025;//025;//5;
float lerpAmount = 0;
int delayTime=5000;
long previousMillis = 0;
boolean changeColor = false;
int mode = 2; // 0 = auto change, 1 = manual, 2 = sound/mic

// OSC: some settings can be changed via osc (eg. iphone). make sure correct ip is used on phone.
#ifdef USE_OSC
OSCServer oscServer;
int oscPort = 5556;
#endif

// net
int ethernetConnected;
boolean hasEthernetBlinked = false;

// ----------------------------------------------------------------------
void setup() {

  // baud rate
  Serial.begin(38400); //19200

    //Serial.println("Memory: Setup begin=");
  //Serial.println(freeMemory());

  // network
#ifdef USE_ETHERNET
  byte myMac[] = { 
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED   };
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
  EthernetBonjour.addServiceRecord("Arduino._ofxBonjourIp",7777,MDNSServiceTCP);
#endif
#endif

  // osc
#ifdef USE_OSC
  int oscConnected = oscServer.begin(oscPort); //ardosc
#ifdef USE_SERIAL_PRINT
  Serial.print("\nOSC connected: ");
  Serial.print(oscConnected);
  Serial.print(" Port: ");
  Serial.print(oscPort);
#endif

  // set callback functions for osc
  oscServer.addCallback("/hueAll",&onHueAll);
  oscServer.addCallback("/hue1",&onHue1);
  oscServer.addCallback("/hue2",&onHue2);
  oscServer.addCallback("/hue3",&onHue3);
  oscServer.addCallback("/hue4",&onHue4);
  oscServer.addCallback("/brightness",&onBrightness);
  oscServer.addCallback("/saturation",&onSaturation);
  oscServer.addCallback("/mode",&onMode);
  oscServer.addCallback("/microphone",&onMicrophone);
  oscServer.addCallback("/micsamples", &onMicSamples); // NEW
  oscServer.addCallback("/micdiff", &onMicDifferenceScale); // NEW
  oscServer.addCallback("/speaker",&onSpeaker);
  oscServer.addCallback("/noisefrequency",&onNoiseFrequency);
#endif

  // inputs + outputs
  pinMode(LED_SERIAL_PIN, OUTPUT); // led serial pin
  pinMode(LED_CLOCK_PIN, OUTPUT); // led clock pin
  pinMode(SPL_PIN, INPUT); // mic pin
  pinMode(SPEAKER_PIN, OUTPUT); // speaker pin
  
  #ifdef USE_NEOPIXELS
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  #endif

  // timers
  int8_t normalTImerId = timer.every(20, onTimerUpdate, 0); // normal update loop
  noiseTimerId = timer.every(noiseFrequencyDelay, onTimerSpeaker, 0); // speaker needs seperate loop

  //Serial.println("Memory: Setup complete=");
  //Serial.println(freeMemory());
#ifdef USE_SERIAL_PRINT
  Serial.println("\nSetup complete");
#endif
}

void loop() {

  // flash the built in led (pin 13) 10 times to signal were connected to network
  // TODO: change this so all the leds initialise green when connected or red when not
  if(!hasEthernetBlinked) {
    if(ethernetConnected == 1) {
      /*for(int i = 0; i < 10; i++) {
        digitalWrite(13, HIGH);
        delay(500); 
        digitalWrite(13, LOW);
        delay(500);
      }*/
      
      // all lights are green - we are connected
      lerpAmount = 0.0;
      for(int i = 0; i < lightCount; i++)  {
        hues[i] = 85;
        ColorUtils::setHsb(newLedRGB[i], hues[i], saturation, brightness);
      }
    } else {
      // all lights are red
      lerpAmount = 0.0;
      for(int i = 0; i < lightCount; i++)  {
        hues[i] = 0;
        ColorUtils::setHsb(newLedRGB[i], hues[i], saturation, brightness);
      }
    }
    
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
  if(!isOn) return;

  // using timers instead of normal loop
  timer.update();

  // sound levels
  //checkMic();

  // speaker - moved 
  //checkSpeaker();

  // rgb leds
  //checkLeds();

  // short delay for serial data and stuff
  // do i need this? might interfere with microphone frequency
  // maybe use timers? http://arduino.cc/playground/Code/Timer
  //delay(10);

  //Serial.println("Memory: loop=");
  //Serial.println(freeMemory());
  //delay(1000);
}

void onTimerUpdate(void *context) {

  if(mode == 0) {
    // automatic mode change colours based on a timer
    autoRandomiseColor();
  } 
  else if(mode == 1) {
    // manually change lights via osc
  } 
  else if(mode == 2) {
    // change lights based on sound input
    updateMic();
  }

  //checkLeds();
  updateLeds();
}

// ------- MIC
void updateMic() {

  if(useMic) {

    // detect sound level, rolling average, and difference
    soundLevel = analogRead(SPL_PIN);
    int avg = getRollingAverage(soundLevel);
    float diff = abs(soundLevel - avg) / soundRange; // normalised 0 - 1.0f

    // 1) sound either turns leds on/off (defined by fadeOnSound), then waits for delay before resetting
    // eg. if baby cries, then the lights go off - this would be a cruel way to train a baby not to cry. 
    /*if(!soundChanged) {
     // check if current read is x amount greater than average
     if(diff > differenceScale)  {       
     soundChanged = true;
     brightness = (fadeOnSound) ? 0 : 255;
     setHsb(newRGB, hue, 255, brightness);
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
    if(!soundChanged) {
      if(diff > differenceScale) {
        soundChanged = true;
        lerpAmount = 0;
        //ColorUtils::setHsb(newRGB, hue, 255, brightness);
        for(int i = 0; i < lightCount; i++)  {
          hues[i] = random(0,255);
          ColorUtils::setHsb(newLedRGB[i], hues[i], saturation, brightness);
        }
      }
    } 
    else {
      // reset after threshold
      soundResetCount++;
      if(soundResetCount > soundResetTimeLimit) {
        soundChanged = false;
        soundResetCount = 0;
        //Serial.println("  time reset");
      }
    }
  }
}

// eg. micReadCount = 120 (number of samples collected)
int getRollingAverage(int v) {
  micSum -= micValues[micPos];  // only need the array to subtract old value
  micSum += v;
  micValues[micPos] = v;     
  //micPos = (micPos + 1) % micReadCount; 
  if (++micPos == micReadCount) micPos = 0;
  return micSum / micReadCount;
}


// ------- SPEAKER
void onTimerSpeaker(void *context) {
  //Serial.println("got speaker?");
  if(useSpeaker) generateNoise();
}


void generateNoise(){
  unsigned long int newr;
  unsigned char lobit;
  unsigned char b31, b29, b25, b24;
  unsigned long int reg = 0x55aa55aaL;
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

  if(lerpAmount < 1) lerpAmount += lerpInc;

  #ifdef USE_NEOPIXELS
  for(int i = 0; i < lightCount; i++)  {
    ColorUtils::lerpRGB(currentLedRGB[i], newLedRGB[i], lerpAmount);
    //uint32_t hexForLed =  ColorUtils::rgbToHex(currentLedRGB[i].r,currentLedRGB[i].g,currentLedRGB[i].b);
    strip.setPixelColor(i, currentLedRGB[i].r,currentLedRGB[i].g,currentLedRGB[i].b);
  }
  strip.show();
  #else
  long mask; //
  for(int i = 0; i < lightCount; i++)  {

    ColorUtils::lerpRGB(currentLedRGB[i], newLedRGB[i], lerpAmount);
    uint32_t hexForLed =  ColorUtils::rgbToHex(currentLedRGB[i].r,currentLedRGB[i].g,currentLedRGB[i].b);

    for(byte color_bit = 23 ; color_bit != 255 ; color_bit--) {
      //Feed color bit 23 first (red data MSB)
      digitalWrite(LED_CLOCK_PIN, LOW); //Only change data when clock is low
      mask = 1L << color_bit;
      //The 1'L' forces the 1 to start as a 32 bit number, otherwise it defaults to 16-bit.
      if(hexForLed & mask) {
        digitalWrite(LED_SERIAL_PIN, HIGH);
      } 
      else {
        digitalWrite(LED_SERIAL_PIN, LOW);
      }

      digitalWrite(LED_CLOCK_PIN, HIGH); //Data is latched when clock goes high
    }
  }

  //Pull clock low to put strip into reset/post mode
  digitalWrite(LED_CLOCK_PIN, LOW);
  #endif
}


void autoRandomiseColor() {
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > delayTime) {
    previousMillis = currentMillis;   
    lerpAmount = 0;    
    //ColorUtils::setHsb(newRGB, hue, saturation, brightness);
    for(int i = 0; i < lightCount; i++)  {
      hues[i] = random(0,255);
      ColorUtils::setHsb(newLedRGB[i], hues[i], saturation, brightness);
    }
  } 
}


// ------- OSC events
#ifdef USE_OSC
void onHue1(OSCMessage *_mes){
  hues[0] = _mes->getArgInt32(0);
  //ColorUtils::setHsb(newRGB, hue, saturation, brightness);
  ColorUtils::setHsb(newLedRGB[0], hues[0], saturation, brightness);
#ifdef USE_SERIAL_PRINT
  Serial.print("\nHue1 changed: ");
  Serial.print(hues[0]);
#endif
}

void onHue2(OSCMessage *_mes){
  hues[1] = _mes->getArgInt32(0);
  //ColorUtils::setHsb(newRGB, hue, saturation, brightness);
  ColorUtils::setHsb(newLedRGB[1], hues[1], saturation, brightness);
#ifdef USE_SERIAL_PRINT
  Serial.print("hue2 changed: ");
  Serial.print(hues[1]);
#endif
}

void onHue3(OSCMessage *_mes){
  hues[2] = _mes->getArgInt32(0);
  //ColorUtils::setHsb(newRGB, hue, saturation, brightness);
  ColorUtils::setHsb(newLedRGB[2], hues[2], saturation, brightness);
#ifdef USE_SERIAL_PRINT
  Serial.print("\nHue3 changed: ");
  Serial.print(hues[2]);
#endif
}

void onHue4(OSCMessage *_mes){
  hues[3] = _mes->getArgInt32(0);
  //ColorUtils::setHsb(newRGB, hue, saturation, brightness);
  ColorUtils::setHsb(newLedRGB[3], hues[3], saturation, brightness);
#ifdef USE_SERIAL_PRINT
  Serial.print("\nHue4 changed: ");
  Serial.print(hues[3]);
#endif
}

void onHueAll(OSCMessage *_mes){
  int hue = _mes->getArgInt32(0);
  //ColorUtils::setHsb(newRGB, hue, saturation, brightness);
  for(int i = 0; i < lightCount; i++)  {
    hues[i] = hue;
    ColorUtils::setHsb(newLedRGB[i], hues[i], saturation, brightness);
  }
#ifdef USE_SERIAL_PRINT
  Serial.print("\nHues changed: ");
  Serial.print(hue);
#endif
}

void onBrightness(OSCMessage *_mes){
  brightness = _mes->getArgInt32(0);
  //ColorUtils::setHsb(newRGB, hue, saturation, brightness);
  for(int i = 0; i < lightCount; i++)  {
    ColorUtils::setHsb(newLedRGB[i], hues[i], saturation, brightness);
  }
#ifdef USE_SERIAL_PRINT
  Serial.print("\nBrightness changed: ");
  Serial.print(brightness);
#endif
}

void onSaturation(OSCMessage *_mes){
  saturation = _mes->getArgInt32(0);
  //ColorUtils::setHsb(newRGB, hue, saturation, brightness);
  for(int i = 0; i < lightCount; i++)  {
    ColorUtils::setHsb(newLedRGB[i], hues[i], saturation, brightness);
  }
#ifdef USE_SERIAL_PRINT
  Serial.print("\nSaturation changed: ");
  Serial.print(saturation);
#endif
}

void onMode(OSCMessage *_mes){
  mode = _mes->getArgInt32(0);
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
  micReadCount = _mes->getArgInt32(0); // 0-120
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
  timer.stop(noiseTimerId);
  noiseTimerId = timer.every(noiseFrequencyDelay, onTimerSpeaker, 0); // speaker needs seperate loop
#ifdef USE_SERIAL_PRINT
  Serial.print("\nNoise frequency changed: ");
  Serial.print(noiseFrequencyDelay);  
#endif
}


void onPower(OSCMessage *_mes){
  isOn = (_mes->getArgInt32(0) == 1) ? true : false;
  for(int i = 0; i < lightCount; i++)  {
    if(!isOn) {
      // switch all lights off- to black
      saturation = 0;
      brightness = 0;
      hues[i] = 0;
      ColorUtils::setHsb(newLedRGB[i], hues[i], saturation, brightness);
      ColorUtils::setHsb(currentLedRGB[i], hues[i], saturation, brightness);
    } 
    else {
      // switch all lights on- to white
      hues[i] = 255;
      saturation = 255;
      brightness = 255;
      ColorUtils::setHsb(newLedRGB[i], hues[i], saturation, brightness);
    }

  }
#ifdef USE_SERIAL_PRINT
  Serial.print("\nPower changed: ");
  Serial.print(isOn);
#endif
}

#endif



// old method for updateing all leds to same color
/*void checkLeds() {
 
 // rgb lerp
 if(lerpAmount < 1) lerpAmount += lerpInc;
 ColorUtils::lerpRGB(currentRGB, newRGB, lerpAmount);
 uint32_t hexForLed =  ColorUtils::rgbToHex(currentRGB.r,currentRGB.g,currentRGB.b);
 
 // light up led
 for(int i = 0; i < lightCount; i++)  {
 for(byte color_bit = 23 ; color_bit != 255 ; color_bit--) {
 //Feed color bit 23 first (red data MSB)
 digitalWrite(CKI, LOW); //Only change data when clock is low
 mask = 1L << color_bit;
 //The 1'L' forces the 1 to start as a 32 bit number, otherwise it defaults to 16-bit.
 if(hexForLed & mask) {
 digitalWrite(SDI, HIGH);
 } 
 else {
 digitalWrite(SDI, LOW);
 }
 
 digitalWrite(CKI, HIGH); //Data is latched when clock goes high
 }
 }
 
 //Pull clock low to put strip into reset/post mode
 digitalWrite(CKI, LOW);
 //delayMicroseconds(500); //Wait for 500us to go into reset
 }*/






