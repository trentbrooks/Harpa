
#pragma once

#include "ofMain.h"
#ifdef _WIN32
#include <functional> // tr1 deprecated in VS
#else
#include <tr1/functional>
#endif

/* 
    EventTimer* timer1 = new EventTimer();
    timer1->start(5.0);
    ofAddListener(timer1->onTimerComplete, this, &testApp::resetNode);
    void testApp::resetNode(int& timeCount) { }
 
    - remember to call timer1->update() from testApp
 */

class EventTimer {
public:    

    float time;
    bool counting;
    float startingTime;
    float timeLimit; 
    
    bool isPaused;
    float pauseTime;
    
    ofEvent<float> onTimerComplete;    
    
    EventTimer() {
        timeLimit = 10; // 10 seconds
        startingTime = 0;
        time = 0;
        pauseTime = 0;
        isPaused = false;
        counting = false;
    };
        
    void update() {
        
        if(isPaused) return;
        if(!counting) return;
          
        time = ofGetElapsedTimef() - startingTime;
        if(time >= timeLimit) {
            counting = false;
            ofNotifyEvent(onTimerComplete,time,this);
        }
    };

    // start the timer.
    void start(float timeLimitSeconds) {
        
        // cannot add new timer when paused.
        if(isPaused) {
            ofLogVerbose() << "! TimerUtils: cannot call EventTimer.startTimer() when timer is paused.";
            return;
        }
        timeLimit = timeLimitSeconds;
        startingTime = ofGetElapsedTimef();
        if(!counting) counting = true;
    };
    
    // restart from previously saved start time.
    void restart() {
        start(timeLimit);
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
				if(counting) {
					float timeDiff = ofGetElapsedTimef() - pauseTime;
					startingTime += timeDiff;
				}
			}
		}
        /*isPaused = doPause;
        if(isPaused) {
            // pause
            pauseTime = ofGetElapsedTimef();
        } else {
            // play- add the time difference since pausing to the start time
            if(counting) {
                float timeDiff = ofGetElapsedTimef() - pauseTime;
                startingTime += timeDiff;
            }
			pauseTime = 0;
        }*/
    };

    void stop() {
        counting = false;
		isPaused = false;
    };

    // get the current timer's time (1,2,3). Optional- get the reverse 'countdown' time (3,2,1).
    float getTime(bool reverseCountdown = false) {
        if(!reverseCountdown)
            return time;
        else
            return timeLimit - time;
    };
    
    bool isCounting() {
        return counting;
    };
    
    // get as minutes + seconds
    string getClockTime(bool reverseCountdown = false) {
        int csec = getTime(reverseCountdown);
        int cmin = int(csec / 60);
        csec = csec % 60;
        return pad(cmin) + ":" + pad(csec);
    };
    
    string pad(int value) {
        if(value < 10) return "0" + ofToString(value);
        return ofToString(value);
    };        
};

/*
    CallbackTimer
    - similar to tween class
    - go() and onDone()
 */

class CallbackTimer {
public:
    
    float time;
    bool counting;
    float startingTime;
    float timeLimit;
    
    bool isPaused;
    float pauseTime;
    
    bool callbackAdded;
    std::tr1::function<void(void)> callback;
    
    CallbackTimer() {
        timeLimit = 10; // 10 seconds
        startingTime = 0;
        pauseTime = 0;
        time = 0;
        isPaused = false;
        counting = false;
        callbackAdded = false;
    };
    
    CallbackTimer& go(float timeLimitSeconds) {
        
        // cannot add new timer when paused.
        if(isPaused) {
            ofLogVerbose() << "! TimerUtils: cannot call CallbackTimer.go() when timer is paused.";
            return *this;
        }
        timeLimit = timeLimitSeconds;
        startingTime = ofGetElapsedTimef();
        if(!counting) counting = true;
        
        return *this;
    };
    
    template <class T>
    void onDone(T  * listener, void (T::*listenerMethod)(void)) {
        if(isPaused) {
            ofLogVerbose() << "! TimerUtils: cannot add CallbackTimer.onDone() listener when timer is paused.";
            return;
        }
        callback = std::tr1::bind(listenerMethod, listener);
        callbackAdded = true;
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
				if(counting) {
					float timeDiff = ofGetElapsedTimef() - pauseTime;
					startingTime += timeDiff;
				}
			}
		}
        /*isPaused = doPause;
        if(isPaused) {
            // pause
            pauseTime = ofGetElapsedTimef();
        } else {
            // play- add the time difference since pausing to the start time
            if(counting) {
                float timeDiff = ofGetElapsedTimef() - pauseTime;
                startingTime += timeDiff;
            }
        }*/
    };
    
    void stop() {
        counting = false;
    };
    
    void reset() {
        time = 0;
        callbackAdded = false;
		isPaused = false;
        stop();
    };
    
    void update() {
        
        if(isPaused) return;
        if(!counting) return;
        
        time = ofGetElapsedTimef() - startingTime;
        if(time >= timeLimit) {
            counting = false;
            if(callbackAdded) {
                callbackAdded = false;
                callback();
            }
        }
    };
    
    bool isCounting() {
        return counting;
    };
};
