#pragma once

#include "ofMain.h"
#include "DisplayLayer.h"
#include "ResourceManager.h"

class MainButton : public DisplayLayer {
public:
    
    MainButton() {
        isUpImage = true;
        blendImage = false;
        doBlend = true;
        oscVal = 0;
        oscAddress = "/";
        blendMode = OF_BLENDMODE_MULTIPLY;
    };
    ~MainButton() {
    };
    
    ofSoundPlayer sound;
    void loadSound(string sndFile) {
        sound.loadSound(sndFile);
    };
    
    string oscAddress;
    int oscVal;
    void setOSCProperties(string address, ofColor clr) {
        oscAddress = address;
        oscVal = clr.getHue();
        ofLog() << oscAddress << ", " << oscVal;
    };
    
    void setOSCProperties(string address, int val) {
        oscAddress = address;
        oscVal = val;
        ofLog() << oscAddress << ", " << oscVal;
    };
    
    bool isUpImage;
    ofTexture* downImage;
    ofTexture* customImage;
    bool hasCustomImage;
    
    bool blendImage;
    bool doBlend;
    ofBlendMode blendMode;
    ofColor blendColor;
    
    void loadImageStates(string upImagePath, string downImagePath) {
        image = ResourceManager::loadTexture(upImagePath);
        width = image->getWidth();
        height = image->getHeight();
        downImage = ResourceManager::loadTexture(downImagePath);
        hasImage = true;
        hasCustomImage = false;
    };
    
    virtual void draw() {
        
        if(!enabled) return;
        
        ofPushMatrix();
        ofPushStyle();
        ofTranslate(position);
        ofSetColor(255,alpha);
        
        if(hasImage) {
            if(!isCentred) {
                //ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
                if (isUpImage) {
                    image->draw(0,0,width,height);
                } else {
                    
                    if(doBlend) ofEnableBlendMode(blendMode);
                    ofSetColor(blendColor);
                    ofRect(0,0,width,height);
                    if(blendImage) {
                        ofSetColor(255,alpha);
                        downImage->draw(0,0,width,height);
                    }                    
                    if(doBlend) ofDisableBlendMode();
                    ofEnableAlphaBlending();
                    
                    
                    ofSetColor(255,alpha);
                    if(hasCustomImage) customImage->draw(0,0,width,height);
                    if(!blendImage)downImage->draw(0,0,width,height);
                }
            } else {
                if (isUpImage) {
                    image->draw(-width*.5,-height*.5,width,height);
                } else {
                    
                    if(doBlend) ofEnableBlendMode(blendMode);
                    ofSetColor(blendColor);
                    ofRect(0,0,width,height);
                    if(blendImage) {
                        ofSetColor(255,alpha);
                        downImage->draw(0,0,width,height);
                    }
                    if(doBlend) ofDisableBlendMode();
                    ofEnableAlphaBlending();
                    ofSetColor(255,alpha);
                    if(!blendImage) downImage->draw(-width*.5,-height*.5,width,height);
                }
            }
            
        }
        customDraw();
        for(int i = 0; i < layers.size(); i++) layers[i]->draw();
        
        ofPopStyle();
        ofPopMatrix();
    };
    
};