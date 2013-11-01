
#pragma  once

#include "ofMain.h"
#ifdef _WIN32
#include <functional> // tr1 deprecated in VS
#else
#include <tr1/functional> 
#endif

// typedef wrappers for the ugly tr1 functions
typedef std::tr1::function<float(float,float,float,float)> EasingFunction;
typedef std::tr1::function<void(void)> CallbackFunction;


//--------------------------------------------------------------
/*
 TweenUtils.h (SingleTween, MultiTween, Vec3fTween)
 - easy tweening with chainable tweens and callbacks.
 - params go(): value to tween pointer, destination for tween value, duration, delay, and ease type
 - remember to call tween.update() in the update;
 
 // sample tween with callback
 MultiTween tween;
 tween.go(&someValuePtr, newValue, durationInSeconds, delayInSeconds, BACK_OUT).onDone(this, &testApp::onSomeTweenCallback));
 */



//--------------------------------------------------------------
/*
 EasingCurves: Tween floats with easing curves.
 - All Penner's easing equations: http://robertpenner.com/easing/
 - All functions are static and take the same 4 params:
 t = current (time/frames) between 0 and duration
 b = start
 c = change amount
 d = duration (time/frames)
 - The returned value from any TweenCurves is the new value.
 */

class EasingCurves {
public:
    
    static float LinearInterpolation(float t,float b , float c, float d){
        return c*t/d + b;
    };
    
    static float QuadraticEaseIn(float t,float b , float c, float d){
        return c*(t/=d)*t + b;
    };    
    
    static float QuadraticEaseOut(float t,float b , float c, float d){
        return -c *(t/=d)*(t-2) + b;
    };
    
    static float QuadraticEaseInOut(float t,float b , float c, float d){
        if ((t/=d/2) < 1) return ((c/2)*(t*t)) + b;
        return -c/2 * (((--t)*(t-2)) - 1) + b;
    };
    
    static float CubicEaseIn(float t,float b , float c, float d){
        return c*(t/=d)*t*t + b;
    };
    
    static float CubicEaseOut(float t,float b , float c, float d){
        return c*((t=t/d-1)*t*t + 1) + b;
    };
    
    static float CubicEaseInOut(float t,float b , float c, float d){
        if ((t/=d/2) < 1) return c/2*t*t*t + b;
        return c/2*((t-=2)*t*t + 2) + b;
    };
    
    static float QuarticEaseIn(float t,float b , float c, float d){
        return c*(t/=d)*t*t*t + b;
    };
    
    static float QuarticEaseOut(float t,float b , float c, float d){
        return -c * ((t=t/d-1)*t*t*t - 1) + b;
    };
    
    static float QuarticEaseInOut(float t,float b , float c, float d){
        if ((t/=d/2) < 1) return c/2*t*t*t*t + b;
        return -c/2 * ((t-=2)*t*t*t - 2) + b;
    };
    
    static float QuinticEaseIn(float t,float b , float c, float d){
        return c*(t/=d)*t*t*t*t + b;
    };
    
    static float QuinticEaseOut(float t, float b , float c, float d){
        return c*((t=t/d-1)*t*t*t*t + 1) + b;
    };
    
    static float QuinticEaseInOut(float t, float b , float c, float d){
        if ((t/=d/2) < 1) return c/2*t*t*t*t*t + b;
        return c/2*((t-=2)*t*t*t*t + 2) + b;
    };
    
    
    static float SineEaseIn(float t, float b , float c, float d){
        return -c * cos(t/d * (PI/2)) + c + b;
    };
    
    static float SineEaseOut(float t, float b , float c, float d){
        return c * sin(t/d * (PI/2)) + b;
    };
    
    static float SineEaseInOut(float t, float b , float c, float d){
        return -c/2 * (cos(PI*t/d) - 1) + b;
    };
    
    static float ExponentialEaseIn(float t, float b , float c, float d){
        return (t==0) ? b : c * pow(2, 10 * (t/d - 1)) + b;
    };
    
    static float ExponentialEaseOut(float t, float b, float c, float d){
        return (t==d) ? b+c : c * (-pow(2, -10 * t/d) + 1) + b;
    };
    
    static float ExponentialEaseInOut(float t, float b , float c, float d){
        if (t==0) return b;
        if (t==d) return b+c;
        if ((t/=d/2) < 1) return c/2 * pow(2, 10 * (t - 1)) + b;
        return c/2 * (-pow(2, -10 * --t) + 2) + b;
    };
    
    static float CircularEaseIn(float t, float b , float c, float d){
        return -c * (sqrt(1 - (t/=d)*t) - 1) + b;
    };
    
    static float CircularEaseOut(float t, float b , float c, float d){
        return c * sqrt(1 - (t=t/d-1)*t) + b;
    };
    
    static float CircularEaseInOut(float t, float b , float c, float d){
        if ((t/=d/2) < 1) return -c/2 * (sqrt(1 - t*t) - 1) + b;
        return c/2 * (sqrt(1 - t*(t-=2)) + 1) + b;
    };
    
    static float BackEaseIn(float t, float b , float c, float d){
        float s = 1.70158f;
        float postFix = t/=d;
        return c*(postFix)*t*((s+1)*t - s) + b;
    };
    
    static float BackEaseOut(float t, float b , float c, float d){
        float s = 1.70158f;
        return c*((t=t/d-1)*t*((s+1)*t + s) + 1) + b;
    };
	
    static float BackEaseInOut(float t, float b , float c, float d){
        float s = 1.70158f;
        if ((t/=d/2) < 1) return c/2*(t*t*(((s*=(1.525f))+1)*t - s)) + b;
        float postFix = t-=2;
        return c/2*((postFix)*t*(((s*=(1.525f))+1)*t + s) + 2) + b;
    };

    
    static float ElasticEaseIn(float t, float b , float c, float d){
        if (t==0) return b;  
        if ((t/=d)==1) return b+c;
        float p=d*.3f;
        float a=c;
        float s=p/4;
        float postFix =a*pow(2,10*(t-=1)); 
        return -(postFix * sin((t*d-s)*(2*PI)/p )) + b;
    };
    
    static float ElasticEaseOut(float t, float b , float c, float d){
        if (t==0) return b;  
        if ((t/=d)==1) return b+c;
        float p=d*.3f;
        float a=c;
        float s=p/4;
        return (a*pow(2,-10*t) * sin( (t*d-s)*(2*PI)/p ) + c + b);
    };
    
    static float ElasticEaseInOut(float t, float b , float c, float d){
        if (t==0) return b;  
        if ((t/=d/2)==2) return b+c;
        float p=d*(.3f*1.5f);
        float a=c;
        float s=p/4;
        
        if (t < 1) {
            float postFix =a*pow(2,10*(t-=1));
            return -.5f*(postFix* sin( (t*d-s)*(2*PI)/p )) + b;
        }
        float postFix =  a*pow(2,-10*(t-=1)); 
        return postFix * sin( (t*d-s)*(2*PI)/p )*.5f + c + b;
    };
    
    
    static float BounceEaseIn(float t, float b , float c, float d){
        return c - BounceEaseOut (d-t, 0, c, d) + b;
    };
	
    static float BounceEaseOut(float t, float b , float c, float d){
        if ((t/=d) < (1/2.75f)) {
            return c*(7.5625f*t*t) + b;
        } else if (t < (2/2.75f)) {
            float postFix = t-=(1.5f/2.75f);
            return c*(7.5625f*(postFix)*t + .75f) + b;
        } else if (t < (2.5/2.75)) {
            float postFix = t-=(2.25f/2.75f);
            return c*(7.5625f*(postFix)*t + .9375f) + b;
        } else {
            float postFix = t-=(2.625f/2.75f);
            return c*(7.5625f*(postFix)*t + .984375f) + b;
        }
    };
    
    static float BounceEaseInOut(float t, float b , float c, float d){
        if (t < d/2) return BounceEaseIn (t*2, 0, c, d) * .5f + b;
        else return BounceEaseOut (t*2-d, 0, c, d) * .5f + c*.5f + b;
    };

};


//--------------------------------------------------------------
// Static array/LUT pointing to all the functions in EasingCurves. Access with an EasingType index.
static EasingFunction EasingFunctions[31] = {EasingCurves::LinearInterpolation, EasingCurves::QuadraticEaseIn, EasingCurves::QuadraticEaseOut, EasingCurves::QuadraticEaseInOut, EasingCurves::CubicEaseIn, EasingCurves::CubicEaseOut, EasingCurves::CubicEaseInOut, EasingCurves::QuarticEaseIn, EasingCurves::QuarticEaseOut, EasingCurves::QuarticEaseInOut, EasingCurves::QuinticEaseIn, EasingCurves::QuinticEaseOut, EasingCurves::QuinticEaseInOut, EasingCurves::SineEaseIn, EasingCurves::SineEaseOut, EasingCurves::SineEaseInOut, EasingCurves::ExponentialEaseIn, EasingCurves::ExponentialEaseOut, EasingCurves::ExponentialEaseInOut, EasingCurves::CircularEaseIn, EasingCurves::CircularEaseOut, EasingCurves::CircularEaseInOut, EasingCurves::BackEaseIn, EasingCurves::BackEaseOut, EasingCurves::BackEaseInOut, EasingCurves::ElasticEaseIn, EasingCurves::ElasticEaseOut, EasingCurves::ElasticEaseInOut, EasingCurves::BounceEaseIn, EasingCurves::BounceEaseOut, EasingCurves::BounceEaseInOut
};

// Types of easing curves (31 total)
enum EasingType {
    
    LINEAR = 0,
    QUADRATIC_IN, QUADRATIC_OUT, QUADRATIC_IN_OUT,
    CUBIC_IN, CUBIC_OUT, CUBIC_IN_OUT,
    QUARTIC_IN, QUARTIC_OUT, QUARTIC_IN_OUT,
    QUINTIC_IN, QUINTIC_OUT, QUINTIC_IN_OUT,
    SINE_IN, SINE_OUT, SINE_IN_OUT,
    EXPONENTIAL_IN, EXPONENTIAL_OUT, EXPONENTIAL_IN_OUT,
    CIRCULAR_IN, CIRCULAR_OUT, CIRCULAR_IN_OUT,
    BACK_IN, BACK_OUT, BACK_IN_OUT,
    ELASTIC_IN, ELASTIC_OUT, ELASTIC_IN_OUT,
    BOUNCE_IN, BOUNCE_OUT, BOUNCE_IN_OUT
};


//--------------------------------------------------------------
// helper class for tweening.
struct TweenSettings {
    
    // tween properties
    float *tweenValue; // pointer to the value we want to tween
    float elapsed; // t always changing- elapsed since startTime
    float from; // b static
    float to; // c static - when settings 'to' remember to subtract 'from' when passing to a TweenCurves function
    float duration; // d static
    float startTime; // when the tween begins
    float delayStartTime, delayDuration; // delay before starting (same thing)
    
    // on finished callback
    bool callbackAdded;
    bool tweenComplete;
	CallbackFunction callback; // void
    
    // easing function
    EasingFunction easeFunc; // float
};


//--------------------------------------------------------------
class SingleTween {
public:
    
    TweenSettings settings;
    bool isReady; // only need this for SingleTween, multi has for loop.
    bool isPaused;
    float pauseTime;
    
    SingleTween() : isReady(false), isPaused(false), pauseTime(0) { };
    
    // GO - main call to perform a tween
    SingleTween& go(float *value, float toVal, float durationVal, float delayVal = 0, EasingType type = QUADRATIC_OUT) {
        
        // cannot add new tweens when paused.
        if(isPaused) {
            ofLogVerbose() << "! TweenUtils: cannot call SingleTween.go() when tweening is paused.";
            return *this;
        }
        
        settings.tweenComplete = false;
        settings.callbackAdded = false;
        
        // subtracting 5 millis from current time to fix error when calling tween.go in update function (barely tweens without it)
        settings.startTime = settings.delayStartTime = ofGetElapsedTimef();// - 0.005;
        settings.elapsed = 0;        
        settings.tweenValue = value; // pointer to value
        settings.from = *value; // from is current position
        settings.to = toVal - settings.from;
        settings.duration = durationVal; // d
        settings.delayDuration = delayVal;
        settings.easeFunc = EasingFunctions[type];
        
        isReady = true;
        
        //return 0; // tween id for a SingleTween is always 0
        return *this; // return reference to self
    };
    
    void pause(bool doPause) {
        
		if(doPause) {
			if(!isPaused) {
				// only pause if not already paused
				isPaused = true;
				pauseTime = ofGetElapsedTimef();
			}
		} else {
			if(isPaused) {
				// only unpause if already paused
				isPaused = false;
				// play- add the time difference since pausing to the start time
				if(isReady) {
					float timeDiff = ofGetElapsedTimef() - pauseTime;
					settings.startTime += timeDiff;
					settings.delayStartTime += timeDiff;
				}
			}
		}

        /*isPaused = doPause;
        if(isPaused) {
            // pause
            pauseTime = ofGetElapsedTimef();
        } else {
            // play- add the time difference since pausing to the start time            
            if(isReady) {
                float timeDiff = ofGetElapsedTimef() - pauseTime;
                settings.startTime += timeDiff;
                settings.delayStartTime += timeDiff;
            }
			pauseTime = 0;
        }  */      
    };
    
    template <class T>
    void onDone(T  * listener, void (T::*listenerMethod)(void)) {
        if(isPaused) {
            ofLogVerbose() << "! TweenUtils: cannot add SingleTween.onDone() listener when tweening is paused.";
            return;
        }
        settings.callback = std::tr1::bind(listenerMethod, listener);
		//settings.callback = std::bind(listenerMethod, listener);
        settings.callbackAdded = true;
    };
    
    void resetAll() {
        isReady = false;
		isPaused = false;
    };
    
    void update() {
        
        if(isPaused) return;
        if(!isReady) return;
        if(!settings.tweenComplete) {
            
            float now = ofGetElapsedTimef(); // using time as measurement
            
            // check if delay has occured
            if(now - settings.delayStartTime > settings.delayDuration) {                
                // normal update: when time has elapsed enough, tween is finished- returns true
                settings.elapsed = now - settings.startTime - settings.delayDuration;                
                if(settings.elapsed > settings.duration) {
                    // tween finished                    
                    *settings.tweenValue = settings.to + settings.from; // force value to target position (EASE IN types are weird)
                    settings.tweenComplete = true;
                    if(settings.callbackAdded) settings.callback();
                } else {
                    // do the tween
                    *settings.tweenValue = settings.easeFunc(settings.elapsed, settings.from, settings.to, settings.duration);
                }
            } else {                
                // if we get passed loop above, means delay has not occured yet
                settings.elapsed = 0;                
            }
        }
    };
};



//--------------------------------------------------------------
class MultiTween : public SingleTween {
public:
    
    vector<TweenSettings> tweens;
    
    MultiTween()  { };
    
    // GO - main call to perform a tween
    MultiTween& go(float *value, float toVal, float durationVal, float delayVal = 0, EasingType type = QUADRATIC_OUT) {
        
        // cannot add new tweens when paused.
        if(isPaused) {
            ofLogVerbose() << "! TweenUtils: cannot call MultiTween.go() when tweening is paused.";
            return *this;
        }
        
        // if tween already exists, erase old one / overwrite.       
        // might make this optional at some point.
        for(int i = 0; i < tweens.size(); i++) {
            if(tweens[i].tweenValue == value) {
                tweens.erase(tweens.begin() + i);
                break;
            }
        }
        
        TweenSettings tweenSettings;
        tweenSettings.tweenComplete = false;
        tweenSettings.callbackAdded = false;
        
        // subtracting 5 millis from current time to fix error when calling tween.go in update function (barely tweens without it)
        tweenSettings.startTime = tweenSettings.delayStartTime = ofGetElapsedTimef();// - 0.005;
        tweenSettings.elapsed = 0;
        tweenSettings.tweenValue = value; // pointer to value
        tweenSettings.from = *value; // from is current position
        tweenSettings.to = toVal - tweenSettings.from;
        tweenSettings.duration = durationVal; // d
        tweenSettings.delayDuration = delayVal;
        tweenSettings.easeFunc = EasingFunctions[type];
        
        tweens.push_back(tweenSettings);        
        return *this;//tweens.size()-1;
    }
    
    void pause(bool doPause) {
        
		if(doPause) {
			if(!isPaused) {
				// only pause if not already paused
				isPaused = true;
				pauseTime = ofGetElapsedTimef();
			}
		} else {
			if(isPaused) {
				// only unpause if already paused
				isPaused = false;
				// play- add the time difference since pausing to the start time
				float timeDiff = ofGetElapsedTimef() - pauseTime;
				for(int i = 0; i < tweens.size(); i++) {
					tweens[i].startTime += timeDiff;
					tweens[i].delayStartTime += timeDiff;
				}
			}
		}
        /*isPaused = doPause;
        if(isPaused) {
            // pause
            pauseTime = ofGetElapsedTimef();
        } else {
            // play- add the time difference since pausing to the start time
			// this doesn't work if not being updated - revisit!
            float timeDiff = ofGetElapsedTimef() - pauseTime;
            for(int i = 0; i < tweens.size(); i++) {
                tweens[i].startTime += timeDiff;
                tweens[i].delayStartTime += timeDiff;
            }
			pauseTime = 0;
        }*/
    };

    // TODO: add overwrite option for tweens.
    // if tween already exists, the tween is overwritten by default (eg. cannot have multiple tweens for the same value).
    /*void setOverwrite(bool doOverwrite) {
        overwrite = doOverwrite;
    };*/
    
    template <class T>
    void onDone(T  * listener, void (T::*listenerMethod)(void)) {
        if(isPaused) {
            ofLogVerbose() << "! TweenUtils: cannot add MultiTween.onDone() listener when tweening is paused.";
            return;
        }
        // will add the listener to the last tween added to the vector
        tweens[tweens.size()-1].callback = std::tr1::bind(listenerMethod, listener);
        tweens[tweens.size()-1].callbackAdded = true;
    }
    
    template <class T>
    void onDone(int tweenId, T  * listener, void (T::*listenerMethod)(void)) {
        if(isPaused) {
            ofLogVerbose() << "! TweenUtils: cannot add MultiTween.onDone() listener when tweening is paused.";
            return;
        }
        tweens[tweenId].callback = std::tr1::bind(listenerMethod, listener);
        tweens[tweenId].callbackAdded = true;
    };
    
    void resetAll() {
        while(tweens.size()) {
            tweens.erase(tweens.begin());
        }
		isPaused = false;
    };
    
    void update() {
        
        if(isPaused) return;        
        for(int i = 0; i < tweens.size(); i++) {
            TweenSettings& tweenSettings = tweens[i];
            if(tweenSettings.tweenComplete) {
                tweens.erase(tweens.begin() + i);
            } else {
                
                float now = ofGetElapsedTimef(); // using time as measurement
                
                // check if delay has occured
                if(now - tweenSettings.delayStartTime > tweenSettings.delayDuration) {                    
                    // normal update: when time has elapsed enough, tween is finished- returns true
                    tweenSettings.elapsed = now - tweenSettings.startTime - tweenSettings.delayDuration;                    
                    if(tweenSettings.elapsed > tweenSettings.duration) {                        
                        // tween finished
                        *tweenSettings.tweenValue = tweenSettings.to + tweenSettings.from; // force value to target position (EASE IN types are weird)
                        tweenSettings.tweenComplete = true;
                        if(tweenSettings.callbackAdded) tweenSettings.callback(); // this shoudln't work?                        
                    } else {
                        // do the tween
                        *tweenSettings.tweenValue = tweenSettings.easeFunc(tweenSettings.elapsed, tweenSettings.from, tweenSettings.to, tweenSettings.duration);
                    }
                } else {
                    // if we get passed loop above, means delay has not occured yet
                    tweenSettings.elapsed = 0;
                }
            }
        }        
    };
    
};

    


//--------------------------------------------------------------
class Vec3fTween : public MultiTween {
public:
    
    Vec3fTween() { };
    
    // GO - main call to perform a tween
    Vec3fTween& go(ofVec3f *vecValues, ofVec3f vecToValues, float durationVal, float delayVal = 0, EasingType type = QUADRATIC_OUT) {
        
        MultiTween::go(&vecValues->x, vecToValues.x, durationVal, delayVal, type);
        MultiTween::go(&vecValues->y, vecToValues.y, durationVal, delayVal, type);
        MultiTween::go(&vecValues->z, vecToValues.z, durationVal, delayVal, type);
        return *this;//tweens.size()-1;
    }
    
};


