#pragma once

#include "ofMain.h"


class ImageUtils {
public:
    
    // this resizes the incoming image- not the output.
    static ofTexture* ResizedTextureFromImage(ofImage *img, int resizeWidth, int resizeHeight, bool useARB = true){
        img->resize(resizeWidth, resizeHeight);
        return TextureFromImage(img, useARB);
    };
    
    // creates a new texture from an ofImage
    static ofTexture* TextureFromImage(ofImage *img, bool useARB = true){
        
        ofTexture* t = new ofTexture();
        if(img->type == OF_IMAGE_COLOR_ALPHA){
            // alpha image
            t->allocate(img->width, img->height, GL_RGBA, useARB);
            t->loadData(img->getPixels(),img->width, img->height, GL_RGBA);
        }else if(img->type == OF_IMAGE_COLOR){
            // rgb image
            t->allocate(img->width, img->height, GL_RGB, useARB);
            t->loadData(img->getPixels(),img->width, img->height, GL_RGB);
        } else {
            // greyscale image
            t->allocate(img->width, img->height, GL_LUMINANCE, false);
            t->loadData(img->getPixels(),img->width, img->height, GL_LUMINANCE);
        }
        
        return t;
    }
    
    // creates a bunch of ofImages (pixels) and then crashes :)
    // test the limits of how much RAM memory the app can handle (see console)
    static void MemoryMaxOutAndCrash() {
        int memoryCount = 100000;
        vector<ofImage*> memoryList;
        for(int i = 0; i < memoryCount; i++) {
            try {
                ofImage* img = new ofImage();
                img->setUseTexture(false);
                //img->loadImage("ui/screen-intro.png");
                img->allocate(1920, 1080, OF_IMAGE_COLOR_ALPHA);
                memoryList.push_back(img);
            } catch(exception& e) {
                ofLogError() << "Memory maxed out at " << i << " 1920x1080 ofImages. Error " << e.what();
                /*for(int j = 0; j < memoryList.size(); j++) {
                    memoryList[i]->clear();
                    delete memoryList[i];
                    memoryList[i] = NULL;
                }
                memoryList.clear();*/
                ofExit();
                break;
            }
        }
    }

};

