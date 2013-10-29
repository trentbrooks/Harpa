#pragma once

#include "ofMain.h"

// scoped lock typedef- (Poco::ScopedLock<ofMutex> & Poco::FastMutex::ScopedLock are the same)
//typedef Poco::ScopedLock<ofMutex> ofScopedLock;
//typedef Poco::FastMutex::ScopedLock ofScopedLock;


/*
 VideoGrabberThread.
 - the only part that is threaded here is the video.update();
 - the copying of pixels and loadData() into a texture is blocking (mutex lock/unlock)
 - if a heavy operation is required (eg. saving buffer or inverting pixels) create another mutex
 */


class VideoGrabberThread : public ofThread {
    
public:
    
    ofVideoGrabber video;
    ofPixels videoPixels;
    ofTexture videoTexture;
    
    int sleepTime, processTime;
    ofMutex timerMutex; // mutex just so we can monitor how long processTime takes
    
    
    //--------------------------------------------------------------
    VideoGrabberThread() {	}
    ~VideoGrabberThread() { waitForThread(); }
    
    
    // thread safe setup, update + draw- to be called from testApp
    void setup(int width, int height, int fps = 60, int deviceId=-1) {
        
        ofLogVerbose() << "VideoThread started: " << width << "w, " << height << "h, " << fps << "fps, " << deviceId;
        processTime = 0;
        sleepTime = 16;
        video.listDevices();
        if(deviceId >= 0) video.setDeviceID(deviceId); //osx = 1, win = 0
        video.setDesiredFrameRate(fps);
        video.initGrabber(width,height);
        video.setUseTexture(false);
        
        videoPixels.allocate(width,height, 3);
        videoTexture.allocate(width, height, GL_RGB);
        startThread(); // true, false = blocking, verbose by default
    }
    
    void update() {
        
        mutex.lock();
        if(videoPixels.isAllocated()) videoTexture.loadData(videoPixels);
        mutex.unlock();
    }
    
    void draw(float x, float y) {
        if(videoTexture.isAllocated()) videoTexture.draw(x, y);
    }
    
    
    //--------------------------------------------------------------
    void threadedFunction(){
        
        while( isThreadRunning() ){
            
            int startTime = ofGetElapsedTimeMillis();
            video.update();
            if(video.isFrameNew()) {
                
                // copy pixels- main mutex
                mutex.lock();
                videoPixels = video.getPixelsRef();
                mutex.unlock();
            }
            
            // a seperate mutex for the timer/fps
            timerMutex.lock();
            processTime = ofGetElapsedTimeMillis() - startTime;
            int diff = sleepTime - processTime; //processTime- 16 = 60fps, 33 = 30fps
            if(diff < 1) diff = 1;
            timerMutex.unlock();
            
            
            sleep(diff);
        }
    }
    
    
    
    //--------------------------------------------------------------
    // getters + setters
    void readToTexture(ofTexture* texture) {
        Poco::ScopedLock<ofMutex> lock(mutex);
        texture->loadData(videoPixels);
    }
    
    unsigned char * getPixels() {
        Poco::ScopedLock<ofMutex> lock(mutex);
        return videoPixels.getPixels();
    }
    
    ofPixels getPixelsRef() {
        Poco::ScopedLock<ofMutex> lock(mutex);
        return videoPixels;
    }
    
    ofTexture& getTextureReference() {
        return videoTexture;
    }
    
    float getProcessingTime() {
        Poco::ScopedLock<ofMutex> lock(timerMutex);
        return processTime;
    }
    
};


