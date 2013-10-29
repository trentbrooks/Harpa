#pragma once

#include "ofMain.h"


/*
 ImageLoaderThread.
 - async image loading (only the loading is threaded)
 - after image is loaded, thread is stopped
 - use in place of ofImage
 */


class ImageLoaderThread : public ofThread {
    
public:
    
    ofImage image;
    string imagePath;
    bool isLoaded;
    
    
    //--------------------------------------------------------------
    ImageLoaderThread() {	}
    ~ImageLoaderThread() { waitForThread(); }
    
    
    void loadImage(string path) {
        isLoaded = false;
        imagePath = path;
        image.clear();
        image.setUseTexture(false);
        startThread();
    }
    
    
    // need to add a draw(x,y,w,h)
    void draw(float x, float y) {
        
        if(!isLoaded) return;
        
        if(!image.isUsingTexture()) {
            image.setUseTexture(true);
            image.reloadTexture();
        }
        
        image.draw(x, y);
    }
    
    
    //--------------------------------------------------------------
    void threadedFunction(){
        
        while( isThreadRunning() ){
            
            mutex.lock();
            if(!isLoaded) {
                image.loadImage(imagePath);
                isLoaded = true;
                stopThread();
            }
            mutex.unlock();
        }
    }
    
    
    
    //--------------------------------------------------------------
    // getters + setters
    ofImage& getImage() {
        Poco::ScopedLock<ofMutex> lock(mutex);
        return image;
    }
    
    unsigned char * getPixels() {
        Poco::ScopedLock<ofMutex> lock(mutex);
        return image.getPixels();
    }
    
    ofPixels getPixelsRef() {
        Poco::ScopedLock<ofMutex> lock(mutex);
        return image.getPixelsRef();
    }
    
    ofTexture& getTextureReference() {
        Poco::ScopedLock<ofMutex> lock(mutex);
        return image.getTextureReference();
    }
    
    
};


