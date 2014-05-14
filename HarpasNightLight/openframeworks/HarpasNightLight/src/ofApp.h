#pragma once

#include "ofMain.h"
#include "ofxiOS.h"
#include "ofxiOSExtras.h"
#include "ofxTouchGUI.h"
#include "ofxBonjourIp.h"
#include "ofxMovieClip.h"
#include "DisplayLayer.h"
#include "ResourceManager.h"
#include "MainButton.h"
#include "TimerUtils.h"
//#include "AlertViewDelegate.h"
#include "ofxiOSBridgeUtils.h"
//#include "ofxLabelView.h"

class ofApp : public ofxiOSApp {
	
    public:
        void setup();
        void update();
        void draw();
        void exit();
	
        void touchDown(ofTouchEventArgs & touch);
        void touchMoved(ofTouchEventArgs & touch);
        void touchUp(ofTouchEventArgs & touch);
        void touchDoubleTap(ofTouchEventArgs & touch);
        void touchCancelled(ofTouchEventArgs & touch);

        void lostFocus();
        void gotFocus();
        void gotMemoryWarning();
        void deviceOrientationChanged(int newOrientation);
    
    // settings
    ofxTouchGUI settings;
    ofxTouchGUITextInput* inputField;
    void onGUIChanged(ofxTouchGUIEventArgs & args);//const void* sender, string &buttonLabel);
    bool doubleTapActivatedMenu;
    
    // images
    ofTexture* wrenchIcon;
    ofTexture* notConnectedIcon;
    bool showNotConnectedIcon;
    
    // connect to arduino via bonjour instead of ip
    ofxBonjourIp bonjour;
    void onPublishedService(const void* sender, string &serviceIp);
    void onDiscoveredService(const void* sender, string &serviceIp);
    void onRemovedService(const void* sender, string &serviceIp);
    bool ignoreBonjour;
    void beginBonjour();
    CallbackTimer bonjourErrorTimer;
    void onBonjourTimeout();
    bool isConnectedViaBonjour;
    
    
    // alerts
    //AlertViewDelegate* alertView;
    void onAlertClosed(ofAlertViewEventArgs& args);//const void* sender, int &buttonId);
    ofxAlertView* alertView;
    ofxLabelView* labelView;
    ofxActivityIndicator* activityIndicatorView;
    
    MultiTween tween;
    void onSettingsHide();
    void onSettingsShow();
    
    // night light settings
    bool isPowerOn;
    bool isMicrophoneEnabled;
    bool isSpeakerEnabled;
    int hueAll;
    int brightness, defaultBrightness;
    int lastSavedBrightness;
    int saturation, defaultSaturation;
    int hue1, hue2, hue3, hue4;
    int colourMode;
    float noiseFrequency;
    int micSamples;
    float micDifference;
    int fadeDelayMillis;
    int numLeds;
    
    // osc
    string host;
    int port;
    string arduinoIpAndPort;
    
    ofImage img;
    
    
    int frameW, frameH;
    //ofxPixelsImageSequenceLoader* loader;
    vector<ofxPixelsImageSequenceLoader*> loaders;
    vector<ofxPixelsMovieClip*> animations;
    
    vector<MainButton*> mainButtons;
    void onMainButtonPressed(DisplayLayerEventArgs& args);//const void* sender, int &buttonId);
};


/*
 server.addCallback("/hueAll",&onHueAll);
 server.addCallback("/hue1",&onHue1);
 server.addCallback("/hue2",&onHue2);
 server.addCallback("/hue3",&onHue3);
 server.addCallback("/hue4",&onHue4);
 server.addCallback("/brightness",&onBrightness);
 server.addCallback("/saturation",&onSaturation);
 server.addCallback("/mode",&onMode);
 server.addCallback("/microphone",&onMicrophone);
 //server.addCallback("/noisefrequency",&onNoiseFreq);
 // POWER
 // speaker
 
 */