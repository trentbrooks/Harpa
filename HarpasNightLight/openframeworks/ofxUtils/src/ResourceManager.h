#pragma once
#include "ofMain.h"


// IMAGES
// be careful to use either ofTextures or ofImages when loading images
// must clear texture/image data manually from your own class
static map<string, ofTexture*> ResourceManagerTextures;
static map<string, ofImage*> ResourceManagerImages;

// FONTS
//TODO: add template function for loading different font types, eg. FontStash or FTGL
static map<string, ofTrueTypeFont*> ResourceManagerFonts;

// VIDEO?
// AUDIO?

class ResourceManager {

	public:

		//--------------------------
		ResourceManager() {	}
        ~ResourceManager() { }

        static ofTexture* loadTexture(string path) {
            
            map<string, ofTexture*>::iterator iter = ResourceManagerTextures.find(path);
            if (iter != ResourceManagerTextures.end() ) {
                // return texture
                return iter->second;
            }

            ofTexture* texture = new ofTexture();
            bool loaded = ofLoadImage(*texture, path);
            ResourceManagerTextures[path] = texture;
            ofLogVerbose("ResourceManager") << path << " loaded ofTexture " << (ofToString( (loaded) ? "successfully." : "failed."));
            return texture;
        };
    
        /*static void clearTextureList() {
            ResourceManagerTextures.clear();            
            ofLogVerbose("ResourceManager") << "Cleared texture list: " << ResourceManagerTextures.size() << ". Must clear individual texture data seperately";
        };*/
    
        static ofImage* loadImage(string path) {
            
            map<string, ofImage*>::iterator iter = ResourceManagerImages.find(path);
            if (iter != ResourceManagerImages.end() ) {
                // return image                
                return iter->second;
            }
            
            ofImage* image = new ofImage();            
            bool loaded = image->loadImage(path);
            ResourceManagerImages[path] = image;
            ofLogVerbose("ResourceManager") << path << " loaded ofImage " << (ofToString( (loaded) ? "successfully." : "failed."));
            return image;
        };
    
        /*static void clearImageList() {
            ResourceManagerImages.clear();
            ofLogVerbose("ResourceManager") << "Cleared image list: " << ResourceManagerImages.size() << ". Must clear individual image data seperately";
        };*/
    
        static ofTrueTypeFont* loadFont(string path, int fontSize) {
            
            map<string, ofTrueTypeFont*>::iterator iter = ResourceManagerFonts.find(path);
            if (iter != ResourceManagerFonts.end() ) {
                // return font                
                return iter->second;
            }
            
            ofTrueTypeFont* font = new ofTrueTypeFont();
            bool loaded = font->loadFont(path, fontSize);
            ofLogVerbose("ResourceManager") << path << " loaded ofTrueTypeFont " << (ofToString( (loaded) ? " successfully." : "failed."));
            return font;
        };
};
