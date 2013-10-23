#include "ofApp.h"



string keyboardText = "";
//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(30);
    ofEnableAlphaBlending();
    ofBackground(230);    
    ofSetOrientation(OF_ORIENTATION_90_LEFT);
    
    ofAppiOSWindow* window = (ofAppiOSWindow*)ofGetWindowPtr();
//    window->enableHardwareOrientation();
//    bool doesHWOrientation = ofGetWindowPtr()->doesHWOrientation();
//    ofLog() << "does hw rot: " << doesHWOrientation;
    
    // defaults
    isPowerOn = true;
    isMicrophoneEnabled = false;
    isSpeakerEnabled = false;
    hueAll = 60; // yewllow
    brightness = 255;
    saturation = 255;
    hue1 = hue2 = hue3 = hue4 = hueAll;
    colourMode = 0;
    port = 5556;
    host = "10.0.1.131";
    string colourModeOptions[] = {"1. Auto change colours", "2. Don't change colours", "3. Sound changes colours"};
    //string description = "ofxTouchGUI includes slider, dropdown list, button/image button, toggle button, text/title fields, input text (ios only atm), and general fixed variables. All items are custom positioned/sized on creation. Colours, fonts, etc can be changed. Settings can be saved to XML. Values can be sent via OSC.";
    
    
    
    // settings
    settings.loadSettings("settings.xml", false, false); // savefile, default font, use mouse (true for mouse, false for multitouch)
    settings.loadFonts("fonts/SueEllenFrancisco.ttf", "fonts/SueEllenFrancisco.ttf", 18, 24); // optional
    settings.setupSendOSC(host, port); // optional (send to desktop machine running ofxTouchGUIExample)
    //settings.setupReceiveOSC(5555); // optional (receives from desktop machine running ofxTouchGUIExample)
    //settings.setBackgroundColor(ofColor(20,200),0,0,340,ofGetHeight());
    settings.setItemSize(300, 35);
    settings.setItemSpacer(5);
    settings.setWindowPosition(ofGetWidth()-settings.getItemWidth()-40, 0);
    settings.setScrollable(true); // optional - new columns will not be auto created, all items add to a single column instead.
    

    
    // add controls
    settings.addTitleText("SETTINGS");
    settings.moveTo(20, 90);
    settings.addToggleButton("POWER", &isPowerOn);
    settings.addToggleButton("MICROPHONE", &isMicrophoneEnabled);
    settings.addToggleButton("SPEAKER", &isSpeakerEnabled);
    settings.disableTouch();
    
    // keyboard input needs special placement, affected by retina, hardware orientation, etc
    float retinaScale = (window->isRetinaEnabled()) ? 0.5: 1.0;
    ofVec2f lastPos = settings.getItemPosition() * retinaScale;
    ofVec2f windowPos = settings.getWindowPosition() * retinaScale;
    int lastItemWidth = settings.getItemWidth() * retinaScale;
    int lastItemHeight = settings.getItemHeight() * retinaScale;
    //settings.moveTo(lastPos.x/2, lastPos.y/2);
    inputField = settings.addTextInput(&host, windowPos.x + lastPos.x, windowPos.y + lastPos.y + lastItemHeight + 5, lastItemWidth, lastItemHeight);
    settings.moveTo(20, 250);
    //settings.addText("LIGHTING");
    settings.addSlider("HUE ALL", &hueAll, 0, 255);
    settings.addSlider("BRIGHTNESS", &brightness, 0, 255);
    settings.addSlider("SATURATION", &saturation, 0, 255);
    settings.addSlider("HUE 1", &hue1, 0, 255);
    settings.addSlider("HUE 2", &hue2, 0, 255);
    settings.addSlider("HUE 3", &hue3, 0, 255);
    settings.addSlider("HUE 4", &hue4, 0, 255);
    settings.addDropDown("COLOUR MODE", 3, &colourMode, colourModeOptions);
    //settings.addText(description);
    settings.addButton("SAVE");
    settings.addButton("RESET");
    settings.addButton("BACK");
    
    settings.newPanel();
    settings.moveTo(settings.getItemWidth() + 4, settings.getItemHeight()-settings.getItemHeight());
    ofxTouchGUIButton* menuBtn = settings.addButton("MENU");
    menuBtn->loadImageStates("ui/settings-up.png", "ui/settings-down.png");
    
    // adds a listener to all gui items, pointing to onGuiChanged();
    settings.addEventListenerAllItems(this);
    //settings.hide();
    
    
    //img.loadImage("ipad.png");
    
    
    
    frameW  = 342;//640;//340;//400;//320;//640;//1024;
    frameH  = 256;//480;//255;//300;//240;//480;//768;
    
    //animations (8)
    int totalAnimations = 9;
    loaders.resize(totalAnimations);
    for(int i = 0; i < loaders.size(); i++) {
        loaders[i] = new ofxPixelsImageSequenceLoader();
    }
    //loader = new ofxPixelsImageSequenceLoader();
    loaders[0]->loadAndCreateSequence("resized-white", "white");
    loaders[1]->loadAndCreateSequence("resized-yellow", "yellow");
    loaders[2]->loadAndCreateSequence("resized-pink", "pink");
    loaders[3]->loadAndCreateSequence("resized-blue", "blue");
    loaders[4]->loadAndCreateSequence("resized-multi", "multi"); // this is shit
    loaders[5]->loadAndCreateSequence("resized-green", "green");
    loaders[6]->loadAndCreateSequence("resized-red", "red");
    loaders[7]->loadAndCreateSequence("resized-orange", "orange");
    loaders[8]->loadAndCreateSequence("resized-black", "black");
    
    
    
    float frameSpeed = 15.0f / ofGetFrameRate();
    animations.resize(totalAnimations);
    mainButtons.resize(totalAnimations);
    for(int i = 0; i < animations.size(); i++) {
        mainButtons[i] = new MainButton();
        //mainButtons[i]->enabled = false;
        mainButtons[i]->setId(i);
        mainButtons[i]->setTouchEnabled(true);
        ofAddListener(mainButtons[i]->onButtonUpEvent, this, &ofApp::onMainButtonPressed);
        animations[i] = new ofxPixelsMovieClip();
        animations[i]->init(loaders[i], frameSpeed);
    }
    
    animations[0]->gotoAndStop("white");
    animations[1]->gotoAndStop("yellow", 3);
    animations[2]->gotoAndStop("pink", 7);
    animations[3]->gotoAndStop("blue", 17);
    animations[4]->gotoAndStop("multi", 14); // multicolour
    animations[5]->gotoAndStop("green", 10);
    animations[6]->gotoAndStop("red", 21);
    animations[7]->gotoAndStop("orange", 24);
    animations[8]->gotoAndStop("black", 27);
    
    mainButtons[0]->loadImageStates("ui/label-empty.png", "ui/label-white.png");
    mainButtons[0]->blendColor = ofColor(255,80);
    mainButtons[0]->doBlend = false;
    mainButtons[0]->setOSCProperties("/brightness", 255); // also set satiration to 0
    mainButtons[0]->loadSound("sound/white.aiff");
    mainButtons[1]->loadImageStates("ui/label-empty.png", "ui/label-yellow.png");
    mainButtons[1]->blendColor = ofColor(255,255,0);//235,212,0);
    mainButtons[1]->setOSCProperties("/hueall", ofColor(255,255,0));
    mainButtons[1]->loadSound("sound/yellow.aiff");
    mainButtons[2]->loadImageStates("ui/label-empty.png", "ui/label-pink.png");
    mainButtons[2]->blendColor = ofColor(255,23,158);
    mainButtons[2]->setOSCProperties("/hueall", ofColor(255,127,0));
    mainButtons[2]->loadSound("sound/pink.aiff");
    mainButtons[3]->loadImageStates("ui/label-empty.png", "ui/label-blue.png");
    mainButtons[3]->blendColor = ofColor(0,187,235);
    mainButtons[3]->setOSCProperties("/hueall", ofColor(0,127,255));
    mainButtons[3]->loadSound("sound/blue.aiff");
    mainButtons[4]->loadImageStates("ui/label-empty.png", "ui/label-multi.png");//ui/label-multicolour6.png");
    mainButtons[4]->doBlend = false;
    mainButtons[4]->blendColor = ofColor(255,0);
    //mainButtons[4]->blendImage = true;
    mainButtons[4]->setOSCProperties("/hueall", ofColor(255)); // need to change this
    mainButtons[4]->loadSound("sound/multicolour.aiff");
    mainButtons[5]->loadImageStates("ui/label-empty.png", "ui/label-green.png");
    mainButtons[5]->blendColor = ofColor(130,252,0);
    mainButtons[5]->setOSCProperties("/hueall", ofColor(0,255,0));
    mainButtons[5]->loadSound("sound/green.aiff");
    mainButtons[6]->loadImageStates("ui/label-empty.png", "ui/label-red.png");
    mainButtons[6]->blendColor = ofColor(234,26,60);
    mainButtons[6]->setOSCProperties("/hueall", ofColor(255,0,0));
    mainButtons[6]->loadSound("sound/red.aiff");
    mainButtons[7]->loadImageStates("ui/label-empty.png", "ui/label-orange.png");
    mainButtons[7]->blendColor = ofColor(255,133,27);
    mainButtons[7]->setOSCProperties("/hueall", ofColor(255,127,0));
    mainButtons[7]->loadSound("sound/orange.aiff");
    mainButtons[8]->loadImageStates("ui/label-empty.png", "ui/label-black.png");
    mainButtons[8]->blendColor = ofColor(89);
    mainButtons[8]->setOSCProperties("/brightness", 0);
    mainButtons[8]->loadSound("sound/black.aiff");
    // if white - send brightness 255 + saturation 0
    // if black - send brightness 0+ saturation 255
    
    /*
     hsb osc options
     - white hue is 0, as is black.
     - multi has no hue, needs to change mode
     */
    
    int yPos = 0;
    int row = 0;
    int maxColumns = 3;
    for (int i = 0; i < animations.size(); i ++) {
        
        int xPos = row* frameW;
        if(i % maxColumns == 0 && i > 0) {
            yPos += frameH;
            xPos = 0;
            row = 0;
        }
        
        animations[i]->setPosition(xPos, yPos);
        mainButtons[i]->setPosition(xPos, yPos,false);
        row++;
    }
}

void ofApp::onGuiChanged(const void* sender, string &buttonLabel) {
    
    // could use the pointer to item that was pressed? eg.
    ofxTouchGUIBase * guiItem = (ofxTouchGUIBase*)sender;
    
    // or just use the label as the identifier
    if(buttonLabel == "SAVE") {
        settings.saveSettings();
    }
    else if(buttonLabel == "RESET") {
        settings.resetDefaultValues();
    }
    else if(buttonLabel == "MENU" || buttonLabel == "BACK") {
        if(settings.activePanel == 1) {
            //for(int i = 0; i < animations.size(); i++) animations[i]->stop();
            //for(int i = 0; i < animations.size(); i++) animations[i]->play();
            settings.showPanel(0);
        } else {
            //for(int i = 0; i < animations.size(); i++) animations[i]->play();
            //for(int i = 0; i < animations.size(); i++) animations[i]->stop();
            settings.showPanel(1);
        }
        //settings.showPanel(0);
    }
    else if(guiItem == inputField) {
        // if host has changed- setup osc again
        //settings.setupSendOSC(host, port);
        if(settings.getHostOSC() != host) {
            ofLog() << "Changed OSC host address: " << host;
            settings.setupSendOSC(host, port);
        }
    }
}

void ofApp::onMainButtonPressed(const void* sender, int &buttonId) {
    
    //for(int i = 0; i < animations.size(); i++) animations[i]->stop();
    //for(int i = 0; i < mainButtons.size(); i++) mainButtons[i]->isUpImage=true ;
    mainButtons[buttonId]->isUpImage = !mainButtons[buttonId]->isUpImage;
    if(!mainButtons[buttonId]->isUpImage) {
        animations[buttonId]->play();
        mainButtons[buttonId]->sound.play();
        // send osc
        // hue
        settings.sendOSC(mainButtons[buttonId]->oscAddress, mainButtons[buttonId]->oscVal);
        // saturation + brightness
        // if white - send brightness 255 + saturation 0
        // if black - send brightness 0+ saturation 255
    } else {
        animations[buttonId]->stop();
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    
    settings.update();
    for(int i = 0; i < mainButtons.size(); i++) mainButtons[i]->update();
        
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    /*ofColor hue;
    hue.setHsb(hueAll, brightness, saturation);
    ofSetColor(hue);
    img.draw(0, 0);*/
    //ofEnableAlphaBlending();
    ofSetColor(255);
    for (int i = 0; i < animations.size(); i ++) {

        animations[i]->drawFrame();
        mainButtons[i]->draw();
    }

	//glBlendFunc(GL_ONE, GL_ONE);
    
    /*ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
    mainButtons[4]->draw();
    ofDisableBlendMode();*/
    
    //ofEnableAlphaBlending();
    // grid lines
    ofSetColor(0,25);
    ofLine(frameW,0, frameW, ofGetHeight()); // y
    ofLine(frameW*2,0, frameW*2, ofGetHeight()); // y
    ofLine(0, frameH, ofGetWidth(), frameH); // x
    ofLine(0, frameH*2, ofGetWidth(), frameH*2); // x
    
    
    // debug
    ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate(), 1), 20,20);
    
    if(settings.activePanel == 0) {
        ofSetColor(0,200);
        ofRect(settings.getWindowPosition().x, settings.getWindowPosition().y, 340, ofGetHeight());
    }
	settings.draw();
}

//--------------------------------------------------------------
void ofApp::exit(){
    
}

//--------------------------------------------------------------
void ofApp::touchDown(ofTouchEventArgs & touch){

    settings.onDown(touch.x, touch.y);
    if(settings.activePanel == 0) return;
    for(int i = 0; i < mainButtons.size(); i++) mainButtons[i]->onDown(touch.x, touch.y);
}

//--------------------------------------------------------------
void ofApp::touchMoved(ofTouchEventArgs & touch){
    settings.onMoved(touch.x, touch.y);
    if(settings.activePanel == 0) return;
    for(int i = 0; i < mainButtons.size(); i++) mainButtons[i]->onMoved(touch.x, touch.y);
}

//--------------------------------------------------------------
void ofApp::touchUp(ofTouchEventArgs & touch){
    settings.onUp(touch.x, touch.y);
    if(settings.activePanel == 0) return;
    for(int i = 0; i < mainButtons.size(); i++) mainButtons[i]->onUp(touch.x, touch.y);
}

//--------------------------------------------------------------
void ofApp::touchDoubleTap(ofTouchEventArgs & touch){
    
}

//--------------------------------------------------------------
void ofApp::touchCancelled(ofTouchEventArgs & touch){
    
}

//--------------------------------------------------------------
void ofApp::lostFocus(){
    
}

//--------------------------------------------------------------
void ofApp::gotFocus(){
    
}

//--------------------------------------------------------------
void ofApp::gotMemoryWarning(){
    
}

//--------------------------------------------------------------
void ofApp::deviceOrientationChanged(int newOrientation){
    
    if((ofOrientation)newOrientation == OF_ORIENTATION_90_LEFT || (ofOrientation)newOrientation == OF_ORIENTATION_90_RIGHT) {
        ofLogVerbose() << "Changing orientation: " << newOrientation;
        ofSetOrientation((ofOrientation)newOrientation);
    }
}
