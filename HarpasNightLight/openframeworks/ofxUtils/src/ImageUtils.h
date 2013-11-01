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
    
};

