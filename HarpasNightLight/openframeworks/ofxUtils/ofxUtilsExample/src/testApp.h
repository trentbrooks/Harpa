#pragma once

#include "ofMain.h"
#include "ofxTouchGUI.h"
#include "MathUtils.h"
#include "MeshUtils.h"
#include "TimerUtils.h"
#include "TweenUtils.h"
#include "ImageUtils.h"
#include "ImageLoaderThread.h"
#include "VideoGrabberThread.h"
#include "DisplayLayer.h"
#include "Poco/Timer.h"



struct SharedData {
    //public:
	int x, y;
};
//	//char str[256];

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    
    
    
    // touch gui
    ofxTouchGUI settings;
    void onGUIChanged(const void* sender, string &buttonLabel);
		
    
    // tweening
    //TweenHandler tween;
    SingleTween tween; // single value tween
    int activeEaseType;
    float duration;
    float tweenVal;
    void onSomeTweenCallback();
    
    MultiTween multiTween;
    Vec3fTween vecTween;
    ofVec3f multiVec;
    bool doTweenPause;
    
    // timer
    CallbackTimer cbTimer;
    EventTimer timer;
    float timeLimit;// = 10;
    void onSomeTimerEvent(float& timeCount);
    void onSomeTimerEvent2();
    ofColor bgColor;
    
    Poco::Timer* pocoTimer;
    //vector<Poco::Timer*> timers;
    void onPocoTimer(Poco::Timer& timer);
    
    // mathutils- double expo smoothing
    float smoothAmount;
    float predictAmount;
    DoubleExpoData smoothData;
    
    int polySmoothingSize;
    
    // meshutils- draw an arc
    void testRibbonRainbow();
    void testRainbows();
    void drawRainbow(ofVec2f centre, float width, float height);
    void drawRainbow2(ofVec2f centre, float width, float height);
    vector<ofPoint> getArcPoints(ofVec2f centre, float radiusX, float radiusY, float startAngle, float endAngle, int arcResolution);
    int arcResolution;
    float colourThickness;// = 20;
    float strokeWidth;
    float startAngle;// = 179;//180;//220;
    float endAngle;// = 361;//320;
    int arcWidth;
    int arcHeight;
    
    bool useCustomArc;
    
    // image + video thread
    ofImage image;
    ImageLoaderThread threadImage;
    VideoGrabberThread videoThread;
    
    // display layers
    DisplayLayer layer;
    void onLayerUpEvent(const void* sender, int& layerId);//ofMessage& msg);
    
    
    // polar/spherical/cartesian
    void drawPolarSphericalCartestian();
    
    
    // drawing a textured triangle
    ofImage testImage;
    vector<ofVec3f> triPoints;//(3);
    vector<ofVec2f> triTexPoints;//(3);
    void drawTexturedTriangle(ofImage &img, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);
    
    
    // random testing shit
    void testBuffers();
    
};
