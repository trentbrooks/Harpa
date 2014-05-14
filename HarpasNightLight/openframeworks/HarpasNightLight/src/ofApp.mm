#include "ofApp.h"


// TODO: add slider for mic samples (0-120) and mic difference scale (0-1.0)
// TODO: will not be using individual leds anymore (60x addressable), so remove individual led adjusting
// TODO: add slider for fade delay
/*
 oscServer.addCallback("/hue",&onHue);
 oscServer.addCallback("/brightness",&onBrightness);
 oscServer.addCallback("/saturation",&onSaturation);
 oscServer.addCallback("/numleds",&onLedCount); // NEW
 oscServer.addCallback("/fadedelay",&onFadeDelay); // NEW
 oscServer.addCallback("/mode",&onMode);
 oscServer.addCallback("/microphone",&onMicrophone);
 oscServer.addCallback("/micsamples", &onMicSamples); // NEW
 oscServer.addCallback("/micdiff", &onMicDifferenceScale); // NEW
 oscServer.addCallback("/speaker",&onSpeaker);
 oscServer.addCallback("/noisefrequency",&onNoiseFrequency);
 */
// possibly program some animations

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(30);
    ofEnableAlphaBlending();
    ofBackground(230);    
    ofSetOrientation(OF_ORIENTATION_90_LEFT);
    ofxiOSDisableIdleTimer();
    
    
    // defaults
    isPowerOn = true;
    isMicrophoneEnabled = false;
    isSpeakerEnabled = false;
    hueAll = 30; // orange
    brightness = defaultBrightness = lastSavedBrightness = 75; //255;
    saturation = defaultSaturation = 255;
    hue1 = hue2 = hue3 = hue4 = hueAll;
    colourMode = 0;
    noiseFrequency = 0.05;
    micSamples = 15;//120;
    micDifference = 0.15;//0.5;
    fadeDelayMillis = 5000;
    numLeds = 30;
    port = 5556;
    host = "10.0.1.131";
    arduinoIpAndPort = host + ":" +ofToString(port);
    isConnectedViaBonjour = false;
    showNotConnectedIcon = true;
    
    
    string colourModeOptions[] = {"NORMAL", "AUTO FADE", "RAINBOW", "SOUND REACTIVE"};
    
    // discover arduino IP via bonjour
    //bonjour.discoverService();
    bonjour.addEventListeners(this);
    //ignoreBonjour = true;
    beginBonjour(); // want to automatically find + connect to arduino
    
    // add a little spinning icon thingy in the middle
    activityIndicatorView = new ofxActivityIndicator();
    //activityIndicatorView->add(true, ofColor(255,255,0));
    
    
    // icon image in top left corner
    //wrenchIcon = ResourceManager::loadTexture("ui/settings-up.png");
    notConnectedIcon = ResourceManager::loadTexture("ui/notconnected.png");
    
    // settings
    settings.loadSettings("settings.xml", false, true); // savefile, default font, use mouse (true for mouse, false for multitouch)
    //settings.loadFonts("fonts/BoB.ttf", "fonts/BoB.ttf", 11, 20); // optional
    //settings.getFont().setLetterSpacing(0.85);
    //settings.getLargeFont().setLetterSpacing(0.75);
    settings.loadFonts("fonts/VAGRoundedStd-Light.otf", "fonts/VAGRoundedStd-Light.otf", 11, 20); // optional
    settings.setupSendOSC(host, port); // optional (send to desktop machine running ofxTouchGUIExample)
    //settings.setupReceiveOSC(5555); // optional (receives from desktop machine running ofxTouchGUIExample)
    settings.setBackgroundColor(ofColor(20,230),0,0,ofGetWidth(),ofGetHeight());
    
    int itemWidth = 300;
    int itemHeight = 36;
    settings.setItemSize(itemWidth, itemHeight);
    settings.setItemSpacer(10);
    //settings.setWindowPosition(ofGetWidth()-settings.getItemWidth()-40, 0);
    //settings.setScrollable(true); // optional - new columns will not be auto created, all items add to a single column instead.
    
    //settings.addVarText("ARDUINO IP", &host)->setTextOffsets(0, 8);
    //settings.addVarText("OSC SEND PORT", &port)->setTextOffsets(0, 8);
    ofColor textClr(20,20,20);
    ofColor yellow1(255,255,0);
    ofColor yellow2(180,180,0);
    ofColor pink1(255,0,150);
    ofColor pink2(150,0,90);
    ofColor cyan1(0,255,255);
    ofColor cyan2(0,190,190);

    
    settings.addText("MICROPHONE:");
    /*ofxTouchGUIToggleButton* micToggle = settings.addToggleButton("MIC ON", &isMicrophoneEnabled);
     micToggle->setBackgroundClrs(yellow2, yellow1, yellow2, yellow1);
     micToggle->setActiveClrs(cyan2);
     micToggle->setTextClr(textClr);
     micToggle->crossX = textClr;
     micToggle->crossLineWidth = 2;
     micToggle->setOSCAddress("/microphone");*/
    ofxTouchGUISlider* micSlider = settings.addSlider("MIC SAMPLES", &micSamples, 1, 120);
    micSlider->setBackgroundClrs(cyan2,cyan1,cyan2,cyan1);
    micSlider->setActiveClrs(pink1);//ofColor(255,255,0));
    micSlider->setTextClr(textClr);
    ofxTouchGUISlider* msd = settings.addSlider("MIC DIFFERENCE", &micDifference, 0, 1.0);
    msd->copyStyle(micSlider);
    msd->setOSCAddress("/micdiff");
    
    settings.addText("SPEAKER:");//->setTextOffsets(0, 2);
    ofxTouchGUIToggleButton* tsb = settings.addToggleButton("SPEAKER ON", &isSpeakerEnabled);
    tsb->setBackgroundClrs(yellow2, yellow1, yellow2, yellow1);
    tsb->setActiveClrs(cyan2);
    tsb->setTextClr(textClr);
    tsb->crossX = textClr;
    tsb->crossLineWidth = 2;
    tsb->setOSCAddress("/speaker");
    settings.addSlider("NOISE FREQUENCY", &noiseFrequency, 0, 0.5)->copyStyle(micSlider); //0.2
    
    
    
    settings.addText("LEDS:");//->setTextOffsets(0, 2);
    //ofxTouchGUIToggleButton* tb = settings.addToggleButton("LEDS ON", &isPowerOn);
    //tb->copyStyle(micToggle);
    //tb->setOSCAddress("/leds");
    
    
    ofTexture* hueSliderImage = ResourceManager::loadTexture("ui/slider-bg2.png");
    ofTexture* hueSliderBall = ResourceManager::loadTexture("ui/slider-fg2.png");
    ofxTouchGUISlider* hs = settings.addSlider("HUE", &hueAll, 0, 255);
    hs->setImageStates(*hueSliderImage, *hueSliderBall);//("ui/slider-bg.png", "ui/slider-fg.png");
    hs->setTextClr(textClr);
    settings.addSlider("BRIGHTNESS", &brightness, 0, 255)->copyStyle(micSlider);
    settings.addSlider("SATURATION", &saturation, 0, 255)->copyStyle(micSlider);
    ofxTouchGUISlider* fd= settings.addSlider("FADE DELAY (AUTO FADE MODE)", &fadeDelayMillis, 0, 20000);
    fd->copyStyle(micSlider);
    fd->setOSCAddress("/fadedelay");
    //ofxTouchGUISlider* ls= settings.addSlider("LED COUNT", &numLeds, 0, 30);
    //ls->copyStyle(micSlider);
    //ls->setOSCAddress("/numleds");
    


    

    

    settings.nextColumn();
    settings.addVarText("OSC", &arduinoIpAndPort);//->setTextOffsets(0, 8);
    ofxTouchGUIButton* b1 = settings.addButton("BONJOUR ARDUINO");
    b1->setBackgroundClrs(yellow1,yellow1, yellow2, yellow2);//cyan1,cyan1,cyan2,cyan2);//pink1,pink1,pink2,pink2);
    b1->setActiveClrs(pink1,pink1,pink2,pink2);
    b1->setTextClr(textClr);
    settings.addButton("ENTER ARDUINO IP")->copyStyle(b1);
    
    settings.addText("MODE:");
    ofxTouchGUIDropDown* d1 = settings.addDropDown("COLOUR MODE", 4, &colourMode, colourModeOptions);
    d1->setActiveClrs(pink1);//pink1,pink1,pink2,pink2);
    d1->setBackgroundClrs(cyan1,cyan1,cyan2,cyan2);
    d1->setTextClr(textClr);
    d1->setArrowClr(textClr);
    d1->setOSCAddress("/mode");
    
    
    ofImage* closeImage = ResourceManager::loadImage("ui/closeBtn.png");
    ofxTouchGUIButton* closeBtn = settings.addButton("CLOSE", ofGetWidth()-44-20, 20);
    closeBtn->setImageStates(*closeImage, *closeImage);
   
    
    //settings.moveTo(690, 440);
//    settings.addText("4X INDIVIDUAL LED COLOURS:")->setTextOffsets(0, 2);
//    //settings.moveTo(690, 486);
//    settings.addSlider("HUE 1", &hue1, 0, 255)->setImageStates(*hueSliderImage, *hueSliderBall);
//    settings.addSlider("HUE 2", &hue2, 0, 255)->setImageStates(*hueSliderImage, *hueSliderBall);
//    settings.addSlider("HUE 3", &hue3, 0, 255)->setImageStates(*hueSliderImage, *hueSliderBall);
//    settings.addSlider("HUE 4", &hue4, 0, 255)->setImageStates(*hueSliderImage, *hueSliderBall);
    
    //settings.addText(description);
    //settings.addButton("SAVE");
    //settings.addButton("RESET");
    //settings.addButton("BACK");
    
    /*settings.newPanel();
    settings.moveTo(settings.getItemWidth() + 4, settings.getItemHeight()-settings.getItemHeight());
    ofxTouchGUIButton* menuBtn = settings.addButton("MENU");
    menuBtn->loadImageStates("ui/settings-up.png", "ui/settings-up.png");*/
    
    // adds a listener to all gui items, pointing to onGuiChanged();
    settings.resetDefaultValues();
    settings.addEventListenerAllItems(this);
    settings.hide();
    
    
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
        ofAddListener(mainButtons[i]->layerUpEvent, this, &ofApp::onMainButtonPressed);
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
    mainButtons[0]->loadSound("sound2/white.aiff");
    mainButtons[1]->loadImageStates("ui/label-empty.png", "ui/label-yellow.png");
    mainButtons[1]->blendColor = ofColor(255,255,0);//235,212,0);
    mainButtons[1]->setOSCProperties("/hue", ofColor(255,255,0));
    mainButtons[1]->loadSound("sound2/yellow.aiff");
    mainButtons[2]->loadImageStates("ui/label-empty.png", "ui/label-pink.png");
    mainButtons[2]->blendColor = ofColor(255,23,158);
    mainButtons[2]->setOSCProperties("/hue", ofColor(255,0,127));
    mainButtons[2]->loadSound("sound2/pink.aiff");
    mainButtons[3]->loadImageStates("ui/label-empty.png", "ui/label-blue.png");
    mainButtons[3]->blendColor = ofColor(0,187,235);
    mainButtons[3]->setOSCProperties("/hue", ofColor(0,127,255));
    mainButtons[3]->loadSound("sound2/blue.aiff");
    mainButtons[4]->loadImageStates("ui/label-empty.png", "ui/label-multicolour6.png");
    mainButtons[4]->hasCustomImage = true;
    mainButtons[4]->customImage = ResourceManager::loadTexture("ui/label-multi2.png");
    //mainButtons[4]->doBlend = false;
    //mainButtons[4]->blendColor = ofColor(255,0);
    mainButtons[4]->blendImage = true;
    //mainButtons[4]->setOSCProperties("/hue", ofColor(255)); // need to change this
    mainButtons[4]->setOSCProperties("/mode", 2); // rainbow mode!
    mainButtons[4]->loadSound("sound2/rainbow.aiff");
    mainButtons[5]->loadImageStates("ui/label-empty.png", "ui/label-green.png");
    mainButtons[5]->blendColor = ofColor(130,252,0);
    mainButtons[5]->setOSCProperties("/hue", ofColor(0,255,0));
    mainButtons[5]->loadSound("sound2/green.aiff");
    mainButtons[6]->loadImageStates("ui/label-empty.png", "ui/label-red.png");
    mainButtons[6]->blendColor = ofColor(234,26,60);
    mainButtons[6]->setOSCProperties("/hue", ofColor(255,0,0));
    mainButtons[6]->loadSound("sound2/red.aiff");
    mainButtons[7]->loadImageStates("ui/label-empty.png", "ui/label-orange.png");
    mainButtons[7]->blendColor = ofColor(255,133,27);
    mainButtons[7]->setOSCProperties("/hue", ofColor(255,127,0));
    mainButtons[7]->loadSound("sound2/orange.aiff");
    mainButtons[8]->loadImageStates("ui/label-empty.png", "ui/label-black.png");
    mainButtons[8]->blendColor = ofColor(89);
    mainButtons[8]->setOSCProperties("/brightness", 0);
    mainButtons[8]->loadSound("sound2/black.aiff");
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
    
    
    // general alert view
    alertView = new ofxAlertView();
    //alertView->createAlert("Hello", "all my peeps", "ok", "cancel", true, true);
    //alertView->createAlert("Alert Title", "My alert message", "ok");
    ofAddListener(alertView->alertClosedEvent, this, &ofApp::onAlertClosed);
    
    
    // force orange button 'on'
    DisplayLayerEventArgs args(mainButtons[7]);
    onMainButtonPressed(args);
    
    /*ofxActionSheet* as = new ofxActionSheet();
     vector<string> strings;
     strings.push_back("1. Hello");
     strings.push_back("2. Good");
     strings.push_back("3. Bye");
     as->add("boop bob", strings);*/
    
    /*string sampleText = "The UILabel class implements a read-only text view. You can use this class to draw one or multiple lines of static text, such as those you might use to identify other parts of your user interface. The base UILabel class provides support for both simple and complex styling of the label text. You can also control over aspects of appearance, such as whether the label uses a shadow or draws with a highlight. If needed, you can customize the appearance of your text further by subclassing.\n\nThe default content mode of the UILabel class is UIViewContentModeRedraw. This mode causes the view to redraw its contents every time its bounding rectangle changes. You can change this mode by modifying the inherited contentMode property of the class.\n\nNew label objects are configured to disregard user events by default. If you want to handle events in a custom subclass of UILabel, you must explicitly change the value of the userInteractionEnabled property to YES after initializing the object. The UILabel class implements a read-only text view. You can use this class to draw one or multiple lines of static text, such as those you might use to identify other parts of your user interface. The base UILabel class provides support for both simple and complex styling of the label text. You can also control over aspects of appearance, such as whether the label uses a shadow or draws with a highlight. If needed, you can customize the appearance of your text further by subclassing.\n\nThe default content mode of the UILabel class is UIViewContentModeRedraw. This mode causes the view to redraw its contents every time its bounding rectangle changes. You can change this mode by modifying the inherited contentMode property of the class.\n\nNew label objects are configured to disregard user events by default. If you want to handle events in a custom subclass of UILabel, you must explicitly change the value of the userInteractionEnabled property to YES after initializing the object.";
    
    labelView = new ofxLabelView();
    //labelView->printSystemFonts();
    labelView->setFont("CourierNewPSMT", 12);
    labelView->setAlignment(ofxLabelView::LEFT);
    labelView->add(sampleText, 200, 20, 600);//,300);*/
}

void ofApp::onGUIChanged(ofxTouchGUIEventArgs & args) {
    
    // could use the pointer to item that was pressed? eg. //const void* sender, string &buttonLabel
    ofxTouchGUIBase * guiItem = (ofxTouchGUIBase*)args.target;
    string buttonLabel = guiItem->getLabel();
    
    // or just use the label as the identifier
    if(buttonLabel == "SAVE") {
        settings.saveSettings();
    }
    else if(buttonLabel == "RESET") {
        settings.resetDefaultValues();
    }
    else if(buttonLabel == "CLOSE") {
        //ofLog() << "closing pressed!";
        settings.hide();
    }
    /*else if(buttonLabel == "MENU" || buttonLabel == "BACK") {
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
    }*/
    else if(guiItem == inputField) {
        // if host has changed- setup osc again
        //settings.setupSendOSC(host, port);
        arduinoIpAndPort = host + ":" +ofToString(port);
        if(settings.getHostOSC() != host) {
            ofLog() << "Changed OSC host address: " << host;
            settings.setupSendOSC(host, port);
        }
    }
    else if(buttonLabel == "ENTER ARDUINO IP") {
        
        alertView->add("Arduino IP", "Enter Arduino IP address below.", "Ok", "Cancel", true, false); // TODO: last param should be true on ipad
    }
    else if( buttonLabel == "BONJOUR ARDUINO") {
        
        //ignoreBonjour = false;
        
        activityIndicatorView->add(true);
        //[busyAnimation startAnimating];
        //[ofxiPhoneGetGLParentView() addSubview:busyAnimation];
        
        settings.hide();
        //settings.showPanel(1);
        
        bonjourErrorTimer.go(10).onDone(this, &ofApp::onBonjourTimeout);
        //bonjour.discoverService();
        beginBonjour();
    }
    else if( buttonLabel == "COLOUR MODE") {
        
        // when color mode has changed, make sure we reset saturation + brightness
        // also send mode again!
        settings.sendOSC("/saturation", saturation);
        settings.sendOSC("/brightness", brightness);
        settings.sendOSC("/mode", colourMode);
    }
}

void ofApp::beginBonjour() {
    
    //ignoreBonjour = false;
    //bonjourErrorTimer.go(10).onDone(this, &ofApp::onBonjourTimeout);
    bonjour.discoverService();
}

void ofApp::onBonjourTimeout() {
    
    /*UIAlertView * _alert = [[UIAlertView alloc] initWithTitle:@"Could not find Arduino"
                                                      message:@"Try entering IP address manually."
                                                     delegate:nil
                                            cancelButtonTitle:@"Ok"
                                            otherButtonTitles:nil];
    //[_alert addButtonWithTitle:@"Cancel"];
    _alert.alertViewStyle = UIAlertViewStylePlainTextInput;
    UITextField * alertTextField = [_alert textFieldAtIndex:0];
    alertTextField.keyboardType = UIKeyboardTypeNumberPad;
    [_alert show];
    [_alert release];*/
    
    alertView->add("Could not find Arduino", "Try entering IP address manually.", "Ok");
    
    //ignoreBonjour = true;
    
    activityIndicatorView->remove();
    //[busyAnimation stopAnimating];
    //[busyAnimation removeFromSuperview];
    
    //settings.showPanel(0);
    settings.show();
    bonjour.stopDiscoverService();
}

void ofApp::onAlertClosed(ofAlertViewEventArgs& event) { //const void* sender, int &buttonId) {
    
    ofLog() << "received closed event " << event.buttonIndex << ", " << event.text;
    if(event.text.length()>1) {
        //change osc address
        host = event.text;
        arduinoIpAndPort = host + ":" +ofToString(port);
        if(settings.getHostOSC() != host) {
            ofLog() << "Changed OSC host address: " << host;
            settings.setupSendOSC(host, port);
        }
    }
}


void ofApp::onMainButtonPressed(DisplayLayerEventArgs& args) {
    
    int buttonId = args.target->getId();
    
    for(int i = 0; i < animations.size(); i++) animations[i]->stop();
    for(int i = 0; i < mainButtons.size(); i++) mainButtons[i]->isUpImage=true ;
    
    mainButtons[buttonId]->isUpImage = !mainButtons[buttonId]->isUpImage;
    if(!mainButtons[buttonId]->isUpImage) {
        animations[buttonId]->play();
        mainButtons[buttonId]->sound.play();
        
        // saturation + brightness for white + black
        // already sends brightness, but need to send saturation as well
        if(buttonId == 0) {
            
            // if white - send brightness 255 + saturation 0
            //brightness = defaultBrightness;//mainButtons[buttonId]->oscVal;
            //mainButtons[buttonId]->oscVal = brightness;
            //saturation = 0;
            settings.sendOSC("/saturation", 0);// saturation);
            settings.sendOSC("/brightness", brightness);
        } else if(buttonId == 8) {
            
            // if black - send brightness 0+ saturation 255
            //brightness = 0; //mainButtons[buttonId]->oscVal;
            //saturation = 255;
            settings.sendOSC("/saturation", 255);//saturation);
            settings.sendOSC("/brightness", 0);//brightness);
        } else {
            
            // send normal osc message (hue for normal colors, brightness for white/back, mode for raindbow)
            settings.sendOSC(mainButtons[buttonId]->oscAddress, mainButtons[buttonId]->oscVal);
            
            // if saturation is 0 or brightness is 0 when changing hue, it won;t work properly
            // eg. if last selected was white or black
            //if(saturation == 0) {
                //saturation = defaultSaturation;
                settings.sendOSC("/saturation", saturation);
            //}
            //if(brightness == 0) {
                //brightness = defaultBrightness;
                settings.sendOSC("/brightness", brightness);
            //}
            
            // if not the rainbow- set to normal mode
            if(buttonId != 4) {
                colourMode = 0;
                settings.sendOSC("/mode", colourMode);
            }
            
        }

    } else {
        animations[buttonId]->stop();
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    
    //settings.update();
    bonjourErrorTimer.update();
    tween.update();
    for(int i = 0; i < mainButtons.size(); i++) mainButtons[i]->update();
    
    if(doubleTapActivatedMenu) {
        settings.show();
        doubleTapActivatedMenu = false;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    /*ofColor hue;
    hue.setHsb(hue, brightness, saturation);
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
    //ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate(), 1), 20,20);
    
    /*if(!settings.isHidden()) {
        ofSetColor(0,200);
        ofRect(settings.getWindowPosition().x, settings.getWindowPosition().y, 340, ofGetHeight());
    }*/
	settings.draw();
    
    
    //wrenchIcon->draw(ofGetWidth()-wrenchIcon->getWidth(),0);
    if(!isConnectedViaBonjour) {
        ofSetColor(255);
        if(ofGetFrameNum() % 20 == 0) showNotConnectedIcon = !showNotConnectedIcon;
        if(showNotConnectedIcon) notConnectedIcon->draw(0, 0);
    }
    
    
    /*if(!settings.isHidden()) {
        ofSetColor(255,25);
        ofLine(40,ofGetHeight()/2, ofGetWidth()-40, ofGetHeight()/2); // y
    }*/
    
    
    // not connected - show spinning progress bar and message
    //  && !ignoreBonjour
    if(activityIndicatorView->isAdded()) {
        ofSetColor(0, 215);
        ofRect(0, 0, ofGetWidth(), ofGetHeight());
        float hue = sin(ofGetElapsedTimef()/10.0) * 127.5 + 127.5;
        ofSetColor(ofColor::fromHsb(hue,255,255)); // 252,212,0);
        string textMsg = "Searching for Harpas night light...";
        ofRectangle sBounds = settings.getLargeFont().getStringBoundingBox(textMsg, 0, 0);
        settings.drawTitleText(textMsg, ofGetWidth()/2 - (sBounds.width/2), 462);
    }
}

//--------------------------------------------------------------
void ofApp::onPublishedService(const void* sender, string &serviceIp) {
    ofLog() << "Received published service event: " << serviceIp;
}

void ofApp::onDiscoveredService(const void* sender, string &serviceIp) {
    ofLog() << "Received discovered service event: " << serviceIp;
    
    // arduino device was discovered - now reset OSC and go back to settings menu.
    host = serviceIp;
    if(settings.getHostOSC() != host) {
        ofLog() << "Changed OSC host address: " << host;
        settings.setupSendOSC(host, port);
    }
    //inputField->setPlaceHolderText(host);
    
    //settings.showPanel(0);
    //settings.show();
    
    // remove/stop spinning animation
    //if(!ignoreBonjour) {
    activityIndicatorView->remove();
        //[busyAnimation stopAnimating];
        //[busyAnimation removeFromSuperview];
    //}
    
    isConnectedViaBonjour = true;
    
}

void ofApp::onRemovedService(const void* sender, string &serviceIp) {
    ofLog() << "Received removed service event: " << serviceIp;
    
    isConnectedViaBonjour = false;
    // close settings if open
    /*if(settings.activePanel == 0) {
        settings.showPanel(1);
    }
    
    // add/start spinning animation
    if(!ignoreBonjour) {
        [busyAnimation startAnimating];
        [ofxiPhoneGetGLParentView() addSubview:busyAnimation];
    }*/
    
}

//--------------------------------------------------------------
void ofApp::exit(){
    
    //settings.saveSettings();
}

//--------------------------------------------------------------
void ofApp::touchDown(ofTouchEventArgs & touch){

    /*ofxActionSheet* as = new ofxActionSheet();
    vector<string> strings;
    strings.push_back("1. Hello");
    strings.push_back("2. Good");
    strings.push_back("3. Bye");
    as->add("", strings);*/
    
    
    
    //if(!bonjour.isConnectedToService() && !ignoreBonjour) return;
    
    settings.onDown(touch.x, touch.y);
    if(!settings.isHidden()) return;
    
    // double tap top left for settings
    int touchRadius = 100;
    if(touch.x > ofGetWidth() - touchRadius && touch.x < ofGetWidth() && touch.y > 0 && touch.y < touchRadius) {
        return;
    }
    //if (touch.x > ofGetWidth()-wrenchIcon->getWidth() && touch.y < wrenchIcon->getHeight()) return;
    for(int i = 0; i < mainButtons.size(); i++) mainButtons[i]->onDown(touch.x, touch.y);
}

//--------------------------------------------------------------
void ofApp::touchMoved(ofTouchEventArgs & touch){
    
    //if(!bonjour.isConnectedToService() && !ignoreBonjour) return;
    
    settings.onMoved(touch.x, touch.y);
    if(!settings.isHidden()) return;
    
    for(int i = 0; i < mainButtons.size(); i++) mainButtons[i]->onMoved(touch.x, touch.y);
}

//--------------------------------------------------------------
void ofApp::touchUp(ofTouchEventArgs & touch){
    
    //if(!bonjour.isConnectedToService() && !ignoreBonjour) return;
    
    settings.onUp(touch.x, touch.y);
    if(!settings.isHidden()) return;
    
    for(int i = 0; i < mainButtons.size(); i++) mainButtons[i]->onUp(touch.x, touch.y);
}

//--------------------------------------------------------------
void ofApp::touchDoubleTap(ofTouchEventArgs & touch){
    
    //if(!bonjour.isConnectedToService() && !ignoreBonjour) return;

    int hitArea = 100;
    if(touch.x >= ofGetWidth() - hitArea && touch.x <= ofGetWidth() && touch.y >= 0 && touch.y <= hitArea) {
        
        // not using the settings gui button
        // using a double touch event instead in top right in case Harpa accidently presses
        /*if(settings.activePanel == 1) {
            settings.showPanel(0);
        } else {
            settings.showPanel(1);
        }*/
        
        //settings.show();
        doubleTapActivatedMenu = true;
        
        //settings.toggleDisplay();
        /*if(settings.isHidden()) {
            
            //settings.setWindowPosition(ofGetWidth()-settings.getItemWidth()-40, 0);
            settings.show();
            //inputField->hide();
            //settings.setWindowPosition(ofGetWidth(), 0);
            //tween.go(&settings.getWindowPosition().x, ofGetWidth()-settings.getItemWidth()-40, 1.0, 0, QUINTIC_OUT).onDone(this, &ofApp::onSettingsShow);
        } else {
            //inputField->hide();
            //tween.go(&settings.getWindowPosition().x, ofGetWidth(), 1.0, 0, QUINTIC_IN).onDone(this, &ofApp::onSettingsHide);
            settings.hide();
            settings.saveSettings();
        }*/
    }
    
    
}

void ofApp::onSettingsShow() {
    inputField->show(false);
}

void ofApp::onSettingsHide() {    
    settings.hide();
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
