#pragma once
#include "ofMain.h"


// be careful to use either Textures or Images
static map<string, ofTexture*> ResourceManagerTextures;
static map<string, ofImage*> ResourceManagerImages;

class ResourceManager {

	public:

		//--------------------------
		ResourceManager() {	}

        static ofTexture* loadTexture(string path) {
            
            map<string, ofTexture*>::iterator iter = ResourceManagerTextures.find(path);
            if (iter != ResourceManagerTextures.end() ) {
                // return texture
                return iter->second;
            }

            ofTexture* texture = new ofTexture();
            bool loaded = ofLoadImage(*texture, path);
            ResourceManagerTextures[path] = texture;
            ofLogVerbose() << path << " loaded " << (ofToString( (loaded) ? "successfully." : "failed."));
            return texture;
        };
    
        // doesn't work???
        /*static ofImage* loadImage(string path) {
            
            map<string, ofImage*>::iterator iter = ResourceManagerImages.find(path);
            if (iter != ResourceManagerImages.end() ) {
                // return image
                return iter->second;
            }
            
            ofImage* image = new ofImage();
            bool loaded = ofLoadImage(*image, path);
            ResourceManagerImages[path] = image;
            ofLogVerbose() << path << " loaded " << (ofToString( (loaded) ? "successfully." : "failed."));
            return image;
        };*/
    
};
