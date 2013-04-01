
#include "ColorUtils.h" // custom
#include <SPI.h>
#include <Ethernet.h> // version IDE 0022
#include <ArdOSC.h>
#include <Timer.h>

// TIMER
Timer timer;

// -------
// MICROPHONE: reads mic level, compares against average. once loud enough sounds
// detected - it waits a certain amount (eg. 15 seconds) of time before resetting.
int SPL_PIN = A1;   // the SPL output is connected to analog pin 0
int micReadCount = 120; // number of samples for average
int micValues[120]; // same number of samples
float differenceScale = 0.5; // how different the read needs to be from the average (0.0 = same, 1.0 = opposite/very different)
int micSum = 0; // sound levels total
int micPos = 0; // array index
int soundResetCount = 0;
int soundResetTimeLimit = 30; 
boolean soundChanged = false;
boolean fadeOnSound = false;
boolean useMic = true;
int soundLevel = 0; // mic level
float soundRange = 700.0f; // mic sensor range: 0 (min) - 700 (max)

// -------
// SPEAKER: plays white noise or ?
int SPEAKER_PIN = 9;
int noiseFrequencyDelay = 50;
boolean useSpeaker = true;

// -------
// RGB LEDS: fades/lerps between colours. led's are daisy chained. they change automatically
// if mode = 0 (eg. every 5 seconds), or can be changed manually via osc (1) or sound detection (2).
int CKI = 2; // clock pin
int SDI = 3; // serial data pin
uint32_t currentHex = 0xff0000;
uint32_t newHex = 0xffff00;
Color currentRGB = {255,0,0};
Color newRGB = {255,255,0};
int brightness = 50;//255;
int hue = 255;
float lerpInc = 0.0025;//025;//5;
float lerpAmount = 0;
int lightCount = 4; // Number of RGBLED modules connected
int delayTime=5000;
long previousMillis = 0;
boolean changeColor = false;
int mode = 2; // 0 = auto change, 1 = manual, 2 = sound/mic

// -------
// OSC: some settings can be changed via osc (eg. iphone). make sure correct ip is used on phone.
byte myMac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte myIp[]  = { 
  192, 168, 1, 9 }; // not required by arduino software, but good to know for phone/osc
int  serverPort  = 4444;
OSCServer server;


// ----------------------------------------------------------------------
void setup() {

  // baud rate
  Serial.begin(38400); //19200

    // osc/network
  /*Ethernet.begin(myMac); // can also pass ip + mac address: (myMac ,myIp);
   server.begin(serverPort); //ardosc
   
   // set callback functions for osc
   server.addCallback("/tg/slider/hue",&onHue);
   server.addCallback("/tg/slider/brightness",&onBrightness);
   server.addCallback("/tg/dropdown/mode",&onMode);
   server.addCallback("/tg/dropdown/lightcount",&onLightCount);
   server.addCallback("/tg/toggle/microphone",&onMicrophone);
   server.addCallback("/tg/slider/noisefrequency",&onNoiseFreq);
   
   // print arduino local IP address:
   Serial.print("My IP address: ");
   for (byte thisByte = 0; thisByte < 4; thisByte++) {
   Serial.print(Ethernet.localIP()[thisByte], DEC);
   Serial.print("."); 
   }*/

  // inputs + outputs
  pinMode(SDI, OUTPUT); // led serial pin
  pinMode(CKI, OUTPUT); // led clock pin
  pinMode(SPL_PIN, INPUT); // mic pin
  pinMode(SPEAKER_PIN, OUTPUT); // speaker pin

  // timers
  timer.every(20, checkModes); 
  timer.every(.05, checkSpeaker); // speaker needs seperate loop
}

void loop() {
  // using timers instead of normal loop
  timer.update();

  // required for osc
  //server.aviableCheck(); // > 0

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
}

void checkModes() {

  if(mode == 0) {
    // automatic mode change colours based on a timer
    autoRandomiseColor();
  } 
  else if(mode == 1) {
    // manually change lights via osc
  } 
  else if(mode == 2) {
    // change lights based on sound input
    checkMic();
  }

  checkLeds();
}

// ------- MIC
void checkMic() {

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
        hue = random(0,255);
        ColorUtils::setHsb(newRGB, hue, 255, brightness);
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

int getRollingAverage(int v) {
  micSum -= micValues[micPos];  // only need the array to subtract old value
  micSum += v;
  micValues[micPos] = v;     
  //micPos = (micPos + 1) % micReadCount; 
  if (++micPos == micReadCount) micPos = 0;
  return micSum / micReadCount;
}


// ------- SPEAKER
void checkSpeaker() {
  if(useSpeaker) generateNoise();
}

unsigned long int reg = 0x55aa55aaL;
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
  //delayMicroseconds (noiseFrequencyDelay); // Changing this value changes the frequency.
} 


// ------- RGB LEDS
long mask; //
void checkLeds() {

  // rgb lerp
  if(lerpAmount < 1) lerpAmount += lerpInc;
  ColorUtils::lerpRGB(currentRGB, newRGB, lerpAmount);
  uint32_t hexForLed =  ColorUtils::rgbToHex(currentRGB.r,currentRGB.g,currentRGB.b);

  // light up led
  for(int i = 0; i < lightCount; i++)
  {
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
}


unsigned long currentMillis;
void autoRandomiseColor() {
  currentMillis = millis();
  if(currentMillis - previousMillis > delayTime) {
    previousMillis = currentMillis;   
    lerpAmount = 0;
    hue = random(0,255);
    ColorUtils::setHsb(newRGB, hue, 255, brightness);
  } 
}


// ------- OSC events
void onHue(OSCMessage *_mes){
  hue = _mes->getArgInt32(0);
  ColorUtils::setHsb(newRGB, hue, 255, brightness);
  Serial.println("hue changed: ");
  Serial.print(hue);
}

void onBrightness(OSCMessage *_mes){
  brightness = _mes->getArgInt32(0);
  ColorUtils::setHsb(newRGB, hue, 255, brightness);
  Serial.println("brightness changed: ");
  Serial.print(brightness);
}

void onMode(OSCMessage *_mes){
  mode = _mes->getArgInt32(0);
  Serial.println("mode changed: ");
  Serial.print(mode);
}

void onLightCount(OSCMessage *_mes){
  lightCount = _mes->getArgInt32(0) + 1;
  Serial.println("light count changed: ");
  Serial.print(lightCount);
}

void onMicrophone(OSCMessage *_mes){
  useMic = _mes->getArgInt32(0);
  Serial.println("microphone changed: ");
  Serial.print(useMic);
}

void onNoiseFreq(OSCMessage *_mes) {
  noiseFrequencyDelay = _mes->getArgInt32(0);
  Serial.println("noise freq changed: ");
  Serial.print(noiseFrequencyDelay);
}



