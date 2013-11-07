
#include <Ethernet.h> // http://arduino.cc/en/reference/ethernet
#include <EthernetBonjour.h> // https://github.com/neophob/EthernetBonjour
#include <SPI.h>
#include "ColorUtils.h" // custom
#include <ArdOSC.h> // https://github.com/recotana/ArdOSC
#include <Timer.h> // https://github.com/JChristensen/Timer



// TIMER
Timer timer;

// POWER
boolean isOn = true;

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
boolean useMic = false;
int soundLevel = 0; // mic level
float soundRange = 700.0f; // mic sensor range: 0 (min) - 700 (max)

// -------
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

// -------ch
// RGB LEDS: fades/lerps between colours. led's are daisy chained. they change automatically
// if mode = 0 (eg. every 5 seconds), or can be changed manually via osc (1) or sound detection (2).
int LED_CLOCK_PIN = 2; // clock pin
int LED_SERIAL_PIN = 3; // serial data pin
uint32_t currentHex = 0xff0000;
uint32_t newHex = 0xffff00;
//Color currentRGB = {0,0,255}; // from color
//Color newRGB = {255,255,0}; // to color
int lightCount = 4; // Number of RGBLED modules connected
Color currentLedRGB[] = { 
  {
    0,0,0    }
  , {
    0,0,0    }
  , {
    0,0,0    }
  , {
    0,0,0    } 
};
Color newLedRGB[] = { 
  {
    0,255,0    }
  , {
    255,127,0    }
  , {
    255,255,0    }
  , {
    255,0,0    } 
};
int hues[] = {
  0,0,0,0}; // custom per led
int brightness = 255;
int saturation = 255;
float lerpInc = 0.0025;//025;//5;
float lerpAmount = 0;
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
int serverPort  = 5556;
OSCServer server;


// ----------------------------------------------------------------------
void setup() {

  // baud rate
  Serial.begin(38400); //19200

   // network
  /*Ethernet.begin(myMac); // using a default myMac address, can also pass ip + mac address: (myMac ,myIp);
  // print arduino local IP address:
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }*/
  
  // bonjour
  Serial.println("hey -2");
  EthernetBonjour.begin("arduino");
  Serial.println("hey -X");
  EthernetBonjour.addServiceRecord("Arduino._ofxBonjourIp",7777,MDNSServiceTCP);
  Serial.println("hey -1");
  
  // osc
  server.begin(serverPort); //ardosc

  // set callback functions for osc
  server.addCallback("/hueAll",&onHueAll);
  server.addCallback("/hue1",&onHue1);
  server.addCallback("/hue2",&onHue2);
  server.addCallback("/hue3",&onHue3);
  server.addCallback("/hue4",&onHue4);
  server.addCallback("/brightness",&onBrightness);
  server.addCallback("/saturation",&onSaturation);
  server.addCallback("/mode",&onMode);
  server.addCallback("/microphone",&onMicrophone);
  server.addCallback("/speaker",&onSpeaker);
  server.addCallback("/noisefrequency",&onNoiseFrequency);


  // inputs + outputs
  pinMode(LED_SERIAL_PIN, OUTPUT); // led serial pin
  pinMode(LED_CLOCK_PIN, OUTPUT); // led clock pin
  pinMode(SPL_PIN, INPUT); // mic pin
  pinMode(SPEAKER_PIN, OUTPUT); // speaker pin

  // timers
  Serial.println("hey 1");
  timer.every(20, onTimerUpdate, 0); // normal update loop
  noiseTimerId = timer.every(noiseFrequencyDelay, onTimerSpeaker, 0); // speaker needs seperate loop
  Serial.println("hey 2");
}

void loop() {
  
  EthernetBonjour.run();
  Serial.println("hey 3");
  if(!isOn) return;

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

  // use timer instead of below
  //delayMicroseconds (noiseFrequencyDelay); // Changing this value changes the frequency.
} 


// ------- RGB LEDS
long mask; //
// update individual leds
void updateLeds() {

  if(lerpAmount < 1) lerpAmount += lerpInc;

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
}

unsigned long currentMillis;
void autoRandomiseColor() {
  currentMillis = millis();
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
void onHue1(OSCMessage *_mes){
  hues[0] = _mes->getArgInt32(0);
  //ColorUtils::setHsb(newRGB, hue, saturation, brightness);
  ColorUtils::setHsb(newLedRGB[0], hues[0], saturation, brightness);
  Serial.println("hue1 changed: ");
  Serial.print(hues[0]);
}

void onHue2(OSCMessage *_mes){
  hues[1] = _mes->getArgInt32(0);
  //ColorUtils::setHsb(newRGB, hue, saturation, brightness);
  ColorUtils::setHsb(newLedRGB[1], hues[1], saturation, brightness);
  Serial.println("hue2 changed: ");
  Serial.print(hues[1]);
}

void onHue3(OSCMessage *_mes){
  hues[2] = _mes->getArgInt32(0);
  //ColorUtils::setHsb(newRGB, hue, saturation, brightness);
  ColorUtils::setHsb(newLedRGB[2], hues[2], saturation, brightness);
  Serial.println("hue3 changed: ");
  Serial.print(hues[2]);
}

void onHue4(OSCMessage *_mes){
  hues[3] = _mes->getArgInt32(0);
  //ColorUtils::setHsb(newRGB, hue, saturation, brightness);
  ColorUtils::setHsb(newLedRGB[3], hues[3], saturation, brightness);
  Serial.println("hue4 changed: ");
  Serial.print(hues[3]);
}

void onHueAll(OSCMessage *_mes){
  int hue = _mes->getArgInt32(0);
  //ColorUtils::setHsb(newRGB, hue, saturation, brightness);
  for(int i = 0; i < lightCount; i++)  {
    hues[i] = hue;
    ColorUtils::setHsb(newLedRGB[i], hues[i], saturation, brightness);
  }
  Serial.println("hue changed: ");
  Serial.print(hue);
}

void onBrightness(OSCMessage *_mes){
  brightness = _mes->getArgInt32(0);
  //ColorUtils::setHsb(newRGB, hue, saturation, brightness);
  for(int i = 0; i < lightCount; i++)  {
    ColorUtils::setHsb(newLedRGB[i], hues[i], saturation, brightness);
  }
  Serial.println("brightness changed: ");
  Serial.print(brightness);
}

void onSaturation(OSCMessage *_mes){
  saturation = _mes->getArgInt32(0);
  //ColorUtils::setHsb(newRGB, hue, saturation, brightness);
  for(int i = 0; i < lightCount; i++)  {
    ColorUtils::setHsb(newLedRGB[i], hues[i], saturation, brightness);
  }
  Serial.println("saturation changed: ");
  Serial.print(saturation);
}

void onMode(OSCMessage *_mes){
  mode = _mes->getArgInt32(0);
  Serial.println("mode changed: ");
  Serial.print(mode);
}

void onMicrophone(OSCMessage *_mes){
  useMic = _mes->getArgInt32(0);
  Serial.println("microphone changed: ");
  Serial.print(useMic);
}

void onSpeaker(OSCMessage *_mes){
  useSpeaker = _mes->getArgInt32(0);
  Serial.println("speaker changed: ");
  Serial.print(useSpeaker);  
}


void onNoiseFrequency(OSCMessage *_mes){
  noiseFrequencyDelay = _mes->getArgFloat(0);
  timer.stop(noiseTimerId);
  noiseTimerId = timer.every(noiseFrequencyDelay, onTimerSpeaker, 0); // speaker needs seperate loop
  Serial.println("noise frequency changed: ");
  Serial.print(noiseFrequencyDelay);  
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
  Serial.println("power changed: ");
  Serial.print(isOn);
}




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




