#pragma once

#include "ofMain.h"
#include "ResourceManager.h"
#include "TweenUtils.h"

/*
 DisplayLayer.
 - general purpose display/ui layer, can add child layers to self.
 - has an ofTexture, which uses ResourceManager to ensure images are only loaded once throughout the app- loadImage(string)
 - has automatic tweening for position, size, and rotation (todo).
 - top left aligned (0,0)
 
 What it is not...
 - a multitouch layer for dragging/moving, resizing, rotating.
 */

// TODO::add hit testing for interaction, touch/cursor ids, + transition states


class DisplayLayer {
    
public:
    
    bool enabled;
    int layerId;
    
    // images, layers
    ofTexture* image;
    bool hasImage;    
    vector<DisplayLayer*> layers;
    
    // display
    ofVec2f position;
    float height, width;
    float alpha;
    bool isCentred;
    
    // tween
    MultiTween tween;
    EasingType ease;
    float duration, delay;
    
    // interactions
    bool touchEnabled, isTouching;
    ofEvent<int> onButtonUpEvent;

    
    //--------------------------------------------------------------
    DisplayLayer() {
        
        layerId = -1;
        hasImage = false;        
        width = height = 0;
        position.set(0, 0);
        alpha = 255;
        image = NULL;        
        ease = QUINTIC_OUT;
        duration = 0.75;
        delay = 0;
        isCentred = false;
        isTouching = false;
        enabled = true;
        touchEnabled = false;
    };
    
    ~DisplayLayer() {
        tween.resetAll();
        clearImage();
        layers.clear();
    };
    
    virtual void setup() {};
    
    
    //--------------------------------------------------------------
    // image loading + clearing
    void loadImage(string path) {
        clearImage();
        image = ResourceManager::loadTexture(path);
        width = image->getWidth();
        height = image->getHeight();
        hasImage = true;
    };
    
    void clearImage() {
        if(hasImage) {
            image->clear();
            delete image;
            image = NULL;
        }
        hasImage = false;
    };
    
    
    //--------------------------------------------------------------
    // sub layers
    void addLayer(DisplayLayer* layer) {
        layers.push_back(layer);
    };
    
    void addLayerAt(DisplayLayer* layer, int layerIndex) {
        layerIndex = ofClamp(layerIndex, 0, layers.size()-1);
        layers.insert(layers.begin()+layerIndex,layer);
    };
    
    void removeLayer(DisplayLayer* layer) {        
        for(int i = 0; i < layers.size(); i++) {
            if(layers[i] == layer) {
                layers.erase(layers.begin()+i);
                return;
            }
        }
    };
    
    void removeLayerAt(int layerIndex) {
      layerIndex = ofClamp(layerIndex, 0, layers.size()-1);
      layers.erase(layers.begin()+layerIndex);
    };
    

    //--------------------------------------------------------------
    // update + draw
    virtual void update() {        
        
        if(!enabled) return;
        
        tween.update();
        for(int i = 0; i < layers.size(); i++) layers[i]->update();
        
    };
    
    virtual void draw() {
        
        if(!enabled) return;
        
        ofPushMatrix();
        ofPushStyle();
        ofTranslate(position);
        ofSetColor(255,alpha);
        
        if(hasImage) {
            if(!isCentred)
                image->draw(0,0,width,height);
            else
                image->draw(-width*.5,-height*.5,width,height);
        }
        customDraw();
        for(int i = 0; i < layers.size(); i++) layers[i]->draw();
        
        ofPopStyle();
        ofPopMatrix();
    };
    
    virtual void customDraw() { };
    
    
    //--------------------------------------------------------------
    // interaction / hittest
    // ofEvent dispatched when button up/released
    // must call onDown, onMoved, and onUp from testApp or parent layer from mouse or touch events
    // automatically updates layers. Dispatches the top most layer's events only, eg. child layer.
    
    bool hitTest(float x, float y) {
        if(!isCentred) {
            return x > position.x && x < position.x + width && y > position.y && y < position.y + height;
        } else {
            int halfW = width * .5;
            int halfH = height * .5;
            return x > position.x -halfW && x < position.x + halfW && y > position.y -halfH && y < position.y + halfH;
        }
    };
    
    bool onDown(float x, float y) {
        if(!enabled) return false;
        for(int i = 0; i < layers.size(); i++) {
            // only one child layer should receive onDown=true event
            if(layers[i]->onDown(x,y)) return true;
        }        
        if(!touchEnabled) return false;
        if(hitTest(x, y)) {
            isTouching = true;
            return true;
        }        
        return false;
    };
    
    bool onMoved(float x, float y) {
        if(!enabled) return false;
        for(int i = 0; i < layers.size(); i++) {
            if(layers[i]->onMoved(x,y)) return true;
        }
        if(!touchEnabled) return false;
        if(isTouching) {
            if(hitTest(x, y)) return true;
        }
        return false;
    };
    
    bool onUp(float x, float y) {
        if(!enabled) return false;
        for(int i = 0; i < layers.size(); i++) {
            if(layers[i]->onUp(x,y)) return true;
        }
        if(!touchEnabled) return false;
        if(isTouching) {
            if(hitTest(x, y)) {
                isTouching = false;
                //ofMessage msg("up");
                ofNotifyEvent(onButtonUpEvent, layerId, this);
                return true;
            }            
        }
        return false;
    };
    
    //--------------------------------------------------------------
    // getters + setters
    ofTexture* getImage() {
        return image;
    };
    
    
    void setId(int i) {
        layerId = i;
    };
    
    bool getId() {
        return layerId;
    };
    
    void setEnabled(bool e) {
        enabled = e;
    };
    
    bool getEnabled() {
        return enabled;
    };
    
    void setTouchEnabled(bool t) {
        touchEnabled = t;
    };
    
    bool getTouchEnabled() {
        return touchEnabled;
    };
    
    vector<DisplayLayer*>& getLayers() {
        return layers;
    }
    
    void setPosition(int x, int y, bool doTween = true) {
        
        if(doTween) {
            tween.go(&position.x, x, duration, delay, ease);
            tween.go(&position.y, y, duration, delay, ease);
        } else {
            position.set(x,y);
        }
        
    };
    
    ofVec2f getPosition() {
        return position;
    };
    
    void setSize(int w, int h, bool doTween = true) {        
        if(doTween) {
            tween.go(&width, w, duration, delay, ease);
            tween.go(&height, h, duration, delay, ease);
        } else {
            height = h;
            width = w;
        }        
    };
    
    void setWidth(float w, bool doTween = true) {
        if(doTween) {
            tween.go(&width, w, duration, delay, ease);
        } else {
            width = w;
        }
    };
    
    float getWidth() {
        return width;
    };
    
    void setHeight(float h, bool doTween = true) {
        if(doTween) {
            tween.go(&height, h, duration, delay, ease);
        } else {
            height = h;
        }
    };
    
    float getHeight() {
        return height;
    };
    
    void setAlpha(float a, bool doTween = true) {
        if(doTween) {
            tween.go(&alpha, a, duration, delay, ease);
        } else {
            alpha = a;
        }
    };
    
    float getAlpha() {
        return alpha;
    };
    
    // for all transitions
    void setTweenSettings(EasingType ease, float duration, float delay = 0) {
        this->ease = ease;
        this->duration = duration;
        this->delay = delay;
    };
    
    MultiTween& getTween() {
        return tween;
    };
    
    // registration point
    void setCentreAligned(bool centred) {
        isCentred = centred;
    };
    
    
};


