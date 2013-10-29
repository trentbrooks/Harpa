#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup(){

    ofSetFrameRate(60);
    ofEnableAlphaBlending();
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    //ofDisableArbTex();
    ofEnableNormalizedTexCoords();
    
    ofSetSphereResolution(6);
    
    // setup gui
    settings.loadSettings("settings.xml", true, true);
    settings.addText("Tween");
    string easingTypes[] = {"LINEAR","QUADRATIC_IN", "QUADRATIC_OUT", "QUADRATIC_IN_OUT","CUBIC_IN", "CUBIC_OUT", "CUBIC_IN_OUT","QUARTIC_IN", "QUARTIC_OUT","QUARTIC_IN_OUT", "QUINTIC_IN", "QUINTIC_OUT", "QUINTIC_IN_OUT", "SINE_IN", "SINE_OUT", "SINE_IN_OUT", "EXPONENTIAL_IN", "EXPONENTIAL_OUT", "EXPONENTIAL_IN_OUT", "CIRCULAR_IN", "CIRCULAR_OUT", "CIRCULAR_IN_OUT", "BACK_IN", "BACK_OUT", "BACK_IN_OUT", "ELASTIC_IN", "ELASTIC_OUT", "ELASTIC_IN_OUT","BOUNCE_IN", "BOUNCE_OUT", "BOUNCE_IN_OUT"};
    settings.addDropDown("EASING TYPES", 31, &activeEaseType, easingTypes);
    settings.addSlider("TWEEN DURATION", &duration, 0.0f, 10.0f);
    ofxTouchGUIToggleButton* pauseBtn = settings.addToggleButton("PAUSE TWEEN", &doTweenPause);
    ofAddListener(pauseBtn->onChangedEvent, this, &testApp::onGUIChanged);
    settings.addText("Timer");
    settings.addSlider("TIMER LIMIT", &timeLimit, 0.0f, 30.0f);    
    settings.addText("MathUtils DoubleExpoSmoothing");
    settings.addSlider("SMOOTH AMOUNT", &smoothAmount, 0.0f, 1.0f);
    settings.addSlider("PREDICT AMOUNT", &predictAmount, 0.0f, 1.0f);    
    settings.addSlider("POLY SMOOTH SIZE", &polySmoothingSize, 0,20);    
    settings.addText("MeshUtils Arc");
    settings.addSlider("ARC RESOLUTION", &arcResolution, 1, 1200);
    settings.addSlider("ARC WIDTH", &arcWidth, 0, 500);
    settings.addSlider("ARC HEIGHT", &arcHeight, 0, 500);
    settings.addSlider("BAND THICKNESS", &colourThickness, 0, 150);
    settings.addSlider("STROKE WIDTH", &strokeWidth, 0, 24.0f);
    settings.addSlider("START ANGLE", &startAngle, 0, 361);
    settings.addSlider("END ANGLE", &endAngle, 0, 361);
    settings.addToggleButton("USE CUSTOM ARC", &useCustomArc);
    
    
    settings.addText("-");
    ofxTouchGUIButton* saveBtn = settings.addButton("SAVE");
    ofAddListener(saveBtn->onChangedEvent, this, &testApp::onGUIChanged);
    ofxTouchGUIButton* resetBtn = settings.addButton("RESET");
    ofAddListener(resetBtn->onChangedEvent, this, &testApp::onGUIChanged);
    
    
    
    // setup tween
    activeEaseType = BACK_OUT;
    ofLog() << BACK_OUT;
    duration =  1.0; // seconds or frames
    tweenVal = 0;
    tween.go(&tweenVal, ofGetWidth(), duration, 0, (EasingType)activeEaseType).onDone(this, &testApp::onSomeTweenCallback);
    //tween.onDone(this, &testApp::onSomeTweenCallback);
    
    multiVec.set(0,0);
    //multiTween.go(&multiVec.x, ofGetWidth(), duration, 0, (EasingType)activeEaseType);
    //multiTween.go(&multiVec.y, ofGetHeight(), duration, 0, (EasingType)activeEaseType);
    ofVec3f tweenToVec(ofGetWidth()/2,ofGetHeight()/2,0);
    vecTween.go(&multiVec, tweenToVec, duration, 0, (EasingType)activeEaseType);
    
    // setup timer
    timeLimit = 10;// seconds
    timer.start(timeLimit);
    ofAddListener(timer.onTimerComplete, this, &testApp::onSomeTimerEvent);
    bgColor = ofColor(50);
    
    // setup callback timer
    cbTimer.go(5.0).onDone(this, &testApp::onSomeTimerEvent2);
    
    // testing poco timer
    Poco::Timer timer(5000);
    pocoTimer = &timer;
    //pocoTimer = Poco::Timer(5000);// = new Poco::Timer(5000);//,0);
    Poco::TimerCallback<testApp> callback(*this, &testApp::onPocoTimer);
    timer.start(callback);
    
    
    // mathutils- doubleexposmooth
    smoothAmount = 0.9;//0.75;
    predictAmount = 0.85;//0.25;    
    polySmoothingSize = 8;
    
    // meshutils raindow
    arcResolution = 100;
    colourThickness = 20;
    strokeWidth = 6.0f;
    startAngle = 179;//180;//220;
    endAngle = 361;//320;
    arcWidth = 200;
    arcHeight = 200;
    
    useCustomArc = false;
    
    
    // testing layers
    layer.setup();
    layer.loadImage("box-desktop.png");
    layer.setTouchEnabled(true);    
    ofAddListener(layer.onButtonUpEvent, this, &testApp::onLayerUpEvent);

    DisplayLayer* sublayer = new DisplayLayer();
    sublayer->setup();
    sublayer->loadImage("test.png");
    sublayer->setPosition(100,100);
    sublayer->setAlpha(100);
    sublayer->setTweenSettings(BACK_OUT, 1.0, 5.0);
    sublayer->setTouchEnabled(true);
    ofAddListener(sublayer->onButtonUpEvent, this, &testApp::onLayerUpEvent);
    layer.addLayer(sublayer);
    
    // hide the layers/images stuff
    layer.setEnabled(false);
    
    
    // testing algorithms
    ofVec2f somePolarCoord(525,380);
    
    float radius, angle;
    MathUtils::CartesianToPolar(somePolarCoord, radius, angle);
    ofLog() << "Cartesian to polar: (radius,angle) " << radius << ", " << angle;
    
    ofVec2f cart = MathUtils::PolarToCartesian(radius, angle);
    ofLog() << "Polar to cartesian: (x,y) " << cart.x << ", " << cart.y;
    
    
    // random tests
    // testing drawing texture triangle
    testImage.loadImage("test.png");
    triPoints.resize(3);
    triTexPoints.resize(3);
    
    
    // buffer test
    testBuffers();
    
}

// events/callbacks
void testApp::onGUIChanged(const void* sender, string &buttonLabel) {
    // could use the pointer to button that was pressed? eg.
    ofxTouchGUIButton * button = (ofxTouchGUIButton*)sender;
    //cout << buttonLabel << " - " << button->getValue() << endl;
    
    // or just use the label as the identifier
    if(buttonLabel == "SAVE") {
        settings.saveSettings();
    }
    else if(buttonLabel == "RESET") {
        settings.resetDefaultValues();
    }
    else if(buttonLabel == "PAUSE TWEEN") {
        
        // testing pause function for tweens
        // what happens to callbacks, delays, etc?
        //ofLog() << "2. Pausing tween: " << !tween.isPaused;
        //multiTween.pause(!multiTween.isPaused);
        tween.pause(!tween.isPaused);
        //vecTween.pause(!vecTween.isPaused);
        
        // test pause timers as well
        //cbTimer.pause(!cbTimer.isPaused);
        timer.pause(!timer.isPaused);
    }
}

void testApp::onSomeTweenCallback() {
    ofLog() << "callback fool!";
}

void testApp::onSomeTimerEvent(float& timeCount) {
    ofLog() << "timer event fool!";
    timer.start(timeLimit);
    bgColor = ofColor(ofRandom(255),ofRandom(255),ofRandom(255));
}

void testApp::onSomeTimerEvent2() {
    ofLog() << "callback timer event sucker!";
    cbTimer.go(5.0).onDone(this, &testApp::onSomeTimerEvent2);
}

void testApp::onPocoTimer(Poco::Timer& timer) {
    ofLog() << "some poco timer event here";
    //timer.stop();
    
    ofLog() << pocoTimer->skipped();
}

void testApp::onLayerUpEvent(const void* sender, int& layerId) {
    
//ofMessage& msg) {
    
    ofLog() << "on display layer UP ";// << msg.message;
    if(sender == &layer) ofLog() << "WE HAVE A WEINER";
}


//--------------------------------------------------------------
void testApp::update(){
    
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    // putting the new tween in the update loop is farking sloooooooow.
    //if(ofGetFrameNum() % 2 == 0)
    //Tween.go(&tweenVal, ofGetMouseX(), 1, 0, (EasingType)activeEaseType); // global/static
    
    tween.update();
    multiTween.update();
    vecTween.update();
    
    timer.update();    
    cbTimer.update();
    
    MathUtils::DoubleExpoSmoothing(smoothData, smoothAmount, predictAmount);
    
    videoThread.update();
    layer.update();
}

float trigAngle = 0.0;
deque<ofVec3f> doubleMousePts;
ofColor lerpColor;



//--------------------------------------------------------------
void testApp::draw(){

    lerpColor = lerpColor.getLerped(bgColor, .05);
    ofBackground(lerpColor);
    
    // threaded image loader
    ofSetColor(255);
    threadImage.draw(0,0);

    // threaded video grabber
    videoThread.draw(0, 0);
    
    // display layers
    layer.draw();

    
    // tween
    ofSetColor(255,255,0);//
    ofCircle(tweenVal, ofGetHeight() - 20, 15);
    ofDrawBitmapStringHighlight("Tween", tweenVal, ofGetHeight()- 20);
    //ofLog() << "tween : " << tweenVal << ", from: " << tween.settings.from << ", to: " << tween.settings.to << ", target: " <<  (tween.settings.to + tween.settings.from);
    //tween.go(&tweenVa
    
    ofCircle(multiVec.x, multiVec.y, 15);
    ofDrawBitmapStringHighlight("Multi tween", multiVec.x, multiVec.y);
    //ofLog() << multiTween.tweens.size();
    
    
    // mathutils
    ofSetColor(255, 0, 255);
    ofCircle(smoothData.predicted.x, smoothData.predicted.y, 15);
    ofDrawBitmapStringHighlight("DoubleExpoSmooth", smoothData.predicted.x, smoothData.predicted.y);
    
    
    //timer
    ofDrawBitmapStringHighlight("Timer: " + timer.getClockTime(true), ofGetWidth()/2, ofGetHeight()/2);
    
    
    
    /*ofPushMatrix();
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    ofRotateZ(45);
    ofVec2f pt(ofGetMouseX(), ofGetMouseY());
    pt.rotate(45);
    ofSetColor(100);
    ofRect(-50,-50, 100, 100);
    
    
    ofSetColor(ofColor::blue);
    if(MathUtils::isPointInsideRect(pt, ofGetWidth()/2-50, ofGetHeight()/2-50, ofGetWidth()/2+50, ofGetHeight()/2+50) ) {
        ofCircle(pt, 20);
    }
    ofPopMatrix();
    ofSetColor(ofColor::red);
    ofCircle(pt, 20);*/
    
    testRibbonRainbow();
    
    //testRainbows();

    //drawPolarSphericalCartestian();
    
    // gui
    settings.draw();
    
    
    
}


//deque<ofVec3f> doubleMousePts;
void testApp::testRibbonRainbow() {
    
    // smooth follower
    doubleMousePts.push_back(ofVec3f(ofGetMouseX(), ofGetMouseY(), 0));//smoothData.predicted);
    
    ofPolyline polyPoints;
    for(int i = 0; i < doubleMousePts.size(); i++) {
        polyPoints.addVertex(doubleMousePts[i]);
    }
    
    ofPolyline smooth = polyPoints.getSmoothed(polySmoothingSize);
    float strokeSize = strokeWidth * 4;
    
    
    ofMesh followMeshC = MeshUtils::getSplitMeshWithForceFromPath(smooth,strokeSize - (strokeWidth/2), false, 1);
    ofMesh followMeshB = MeshUtils::getSplitMeshWithForceFromPath(smooth,strokeSize - (strokeWidth/2), true, 1);
    ofMesh followMesh = MeshUtils::getMeshWithForceFromPath(smooth,strokeSize * .33 - (strokeWidth/2), .5);
    //ofMesh followMeshC = MeshUtils::getSplitMeshFromPath(smooth,strokeSize - (strokeWidth/2), false);
    //ofMesh followMeshB = MeshUtils::getSplitMeshFromPath(smooth,strokeSize * .66 - (strokeWidth/2), false);
    //ofMesh followMesh = MeshUtils::getSplitMeshFromPath(smooth,strokeSize * .33 - (strokeWidth/2), false);
    
    glLineWidth(strokeWidth);
    ofSetColor(0);
    followMeshC.drawWireframe();
    ofSetColor(0,255,255);
    followMeshC.drawFaces();
    ofSetColor(0);
    followMeshB.drawWireframe();
    ofSetColor(255,0,255);
    followMeshB.drawFaces();
    
    ofSetColor(0);
    followMesh.drawWireframe();
    ofSetColor(255,255,0);
    followMesh.drawFaces();
    
    
    while(doubleMousePts.size() > 200) {
        doubleMousePts.pop_front();
    }
}

void testApp::testRainbows() {
    
    // rainbow
    //int radiusX = 200;//150;
    //int radiusY = 200;
    
    ofVec2f centre(ofGetWidth()/2, ofGetHeight()/2 + arcHeight);//(radiusY/2));
    float processTime = ofGetElapsedTimeMillis();
    for(int i= 0; i < 10; i++) {
        drawRainbow(centre, arcWidth, arcHeight);
    }
    float rainbow1Test = ofGetElapsedTimeMillis() - processTime;
    
    //ofLog() << processTime;
    // alternative to polylive arc
    /*ofSetColor(0);
     vector<ofPoint> pts = getArcPoints(centre, arcWidth, arcHeight, startAngle, endAngle, arcResolution);
     for(int i = 0; i < pts.size(); i++) {
     ofCircle(pts[i].x, pts[i].y, 3);
     }*/
    
    centre.y -= 150;
    processTime = ofGetElapsedTimeMillis();// - processTime;
    for(int i= 0; i < 10; i++) {
        drawRainbow2(centre, arcWidth, arcHeight);
    }
    float rainbow2Test = ofGetElapsedTimeMillis() - processTime;
    ofLog() << "rainbow draw tests... " << rainbow1Test << ", " << rainbow2Test << " : " << (rainbow1Test - rainbow2Test);
}



// testing polar <--> cartesian * spherical <--> cartesian
float radius = 100, angle = 0;
deque<ofVec2f> polarCartPts;

float radius2 = 250;
float polarAngle = PI/2, aziAngle = PI/2; //SphereToCartesian(float radius, float theta, float phi)

void testApp::drawPolarSphericalCartestian() {
    
    
    // polar to cart
    ofVec2f cart = MathUtils::PolarToCartesian(radius, angle);
    polarCartPts.push_back(cart);
    while(polarCartPts.size() > 1500) {
        polarCartPts.pop_front();
    }
    
    angle+= .01;
    radius = ofMap(ofGetMouseX(), 0, ofGetWidth(), 0, 500);// ofNoise(ofGetElapsedTimef()) * 500;
    radius = ofNoise(ofGetElapsedTimef()) * 500;
    ofPushMatrix();
    ofPushStyle();
    ofSetLineWidth(1);
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    
    ofSetColor(ofColor::white);
    ofMesh pcMesh = MeshUtils::getMeshWithForceFromPath(polarCartPts,1, 1);
    pcMesh.drawWireframe();
    for(int i = 0; i < polarCartPts.size(); i++) {
        //ofCircle(polarCartPts[i], 10);
        if(i > 0) ofLine(polarCartPts[i-1].x, polarCartPts[i-1].y,polarCartPts[i].x, polarCartPts[i].y);
    }
    ofSetColor(ofColor::green);
    ofCircle(cart, 10);
    ofLine(0,0,cart.x,cart.y);
    ofPopStyle();
    ofPopMatrix();
    
    
    // sphere to cartesian (3D)
    glEnable(GL_DEPTH_TEST);
    ofPushMatrix();
    ofPushStyle();
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    ofVec3f cart3D = MathUtils::SphereToCartesian(radius2, polarAngle, aziAngle);
    aziAngle += .01;
    polarAngle += .01;
    //ofCircle(cart3D, 20);
    ofSetColor(ofColor::red);
    ofNoFill();
    ofSphere(cart3D, 20);
    ofLine(0, 0, 0, cart3D.x, cart3D.y, cart3D.z);
    //float radius2 = 200, polarAngle = PI/2, aziAngle = PI/3; //SphereToCartesian(float radius, float theta, float phi)
    
    /*float r, p, a; //output
     MathUtils::CartesianToSphere(cart3D, r, p, a);
     ofLog() << "xxx " << polarAngle << ", " << p;
     ofPopStyle();
     ofPopMatrix();*/
    
    
    // drawing a textured triangle
    float cx = ofGetWidth() * .5;
    float cy = ofGetHeight() * .5;
    drawTexturedTriangle(testImage, cx, cy-50, 0, cx - 50, cy + 50, 0, cx + 50, cy + 50, 0);
    glDisable(GL_DEPTH_TEST);
}

// this is slower than the polyline/path version
vector<ofPoint> testApp::getArcPoints(ofVec2f centre, float radiusX, float radiusY, float startAngle, float endAngle, int arcResolution) {
    
    
    int index = 0;
    float startRad = ofDegToRad(startAngle);
    float endRad = ofDegToRad(endAngle);
    float res = ofDegToRad(360.0f/arcResolution);
    int resizeTo = (endRad - startRad) / res + 1;
    vector<ofPoint> points(resizeTo);
    //points.reserve(resizeTo);
    for(float i = startRad; i < endRad; i += res) {
    //for(int i = 0; i < arcResolution; i++) {
        //  drawpixel(50 + Math.cos(i) * r, y: 100 + Math.sin(i) * r); // center point is (50,100)

        //float angle = (i * ofDegToRad(endAngle)) / arcResolution;// - ofDegToRad(startAngle) ; //-ofDegToRad(startAngle) +
        //points.push_back(ofPoint(cos(i) * radiusX + centre.x,sin(i) * radiusY + centre.y));
        points[index].x = cos(i) * radiusX + centre.x;
        points[index].y = sin(i) * radiusY + centre.y;
        index++;
    }
    //ofLog() << points.size() << ", " << resizeTo;
    return points;
}

void testApp::drawRainbow(ofVec2f centre, float width, float height) {
    
    // colours
    int colourCount = 3;
    ofColor colours[] = { ofColor(255,255,0), ofColor(0,255,255), ofColor(255,0,255)};
    
    
    glLineWidth(strokeWidth);
    glEnable(GL_LINE_SMOOTH); // this is SLOW when no antialiasing on!
    
    // create an arc
    ofPolyline curve;
    for(int i = 0; i < colourCount; i++) {
        ofMesh mesh;
        if(useCustomArc) {
            vector<ofPoint> pts = getArcPoints(centre, width, height, startAngle, endAngle, arcResolution);
            mesh = MeshUtils::getMeshFromPath(pts,colourThickness);
        } else {
            curve.clear();
            curve.arc(centre, width, height, startAngle, endAngle, arcResolution);
            mesh = MeshUtils::getMeshFromPath(curve,colourThickness);
        }
        
        //vector<ofPoint> points = getArcPoints(centre, width, height, startAngle, endAngle, arcResolution);
        //ofMesh mesh = MeshUtils::getMeshFromPath(curve,colourThickness);
        
        ofSetColor(colours[i]);
        mesh.drawFaces();
        ofSetColor(0);
        mesh.drawWireframe();
        // prep next raindow band
        width += (colourThickness * 2);// + (strokeWidth*.5);
        height += (colourThickness * 2);// + (strokeWidth*.5);
        //startAngle += -1.5;
        //endAngle += -.6;
    }

    /*
    // create a thick mesh ribbon from path
    // last segment is not drawn?
    float thickness = 20;
    float lineWidth = 6;
    ofMesh mesh = MeshUtils::getMeshFromPath(curve,thickness);
    
    ofSetColor(0);
    glLineWidth(lineWidth);
    glEnable(GL_LINE_SMOOTH);
    mesh.drawWireframe();
    ofSetColor(255,255,0);
    mesh.drawFaces();
    
    //ofSetColor(0);
    //curve.draw();
    
    //centre.y -= 20;
    width += (thickness * 2) + (lineWidth*.5);// = 193;
    height += (thickness * 2) + (lineWidth*.5);//243;
    curve.clear();
    curve.arc(centre, width, height, 219, 320.5, arcResolution);
    
    ofMesh mesh2 = MeshUtils::getMeshFromPath(curve,thickness);
    ofSetColor(0);
    glLineWidth(lineWidth);
    glEnable(GL_LINE_SMOOTH);
    mesh2.drawWireframe();
    ofSetColor(0,255,255);
    mesh2.drawFaces();
    
    
    width += (thickness * 2) + (lineWidth*.5);// = 193;
    height += (thickness * 2) + (lineWidth*.5);//243;
    curve.clear();
    curve.arc(centre, width, height, 218, 321, arcResolution);
    
    ofMesh mesh3 = MeshUtils::getMeshFromPath(curve,thickness);
    ofSetColor(0);
    glLineWidth(lineWidth);
    glEnable(GL_LINE_SMOOTH);
    mesh3.drawWireframe();
    ofSetColor(255,0,255);
    mesh3.drawFaces();*/
}


void testApp::drawRainbow2(ofVec2f centre, float width, float height) {
    
    
    // colours
    int colourCount = 3;
    ofColor colours[] = { ofColor(255,255,0), ofColor(0,255,255), ofColor(255,0,255)};
    
    
    //glLineWidth(strokeWidth);
    //glEnable(GL_LINE_SMOOTH); // this is SLOW when no antialiasing on!
    
    ofSetColor(255);
    
    float thick = colourThickness * 2;
    ofPath curve;
    curve.setArcResolution(arcResolution);
    for(int i = 0; i < colourCount; i++) {
        
        
        curve.arc(centre, width, height, startAngle, endAngle);
        curve.arcNegative(centre, width-thick, height-thick, endAngle, startAngle);
        curve.close();
        
        //ofSetColor(colours[i]);
        
        curve.setFillColor(colours[i]);
        curve.setStrokeWidth(strokeWidth);
        curve.setStrokeColor(ofColor(0));
        //curve.setFilled(true);
        
        curve.draw();
        curve.clear();
        width += (thick);// * 2);// + (strokeWidth*.5);
        height += (thick);// * 2);// + (strokeWidth*.5);
        
    }
    

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if(key == ' ')
        settings.toggleDisplay();
    else if(key == 'q')
        // threaded image
        threadImage.loadImage("test.png");
    else if(key == 'a')
        // threaded video
        videoThread.setup(640, 480);
    
    
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    smoothData.target.x = x;
    smoothData.target.y = y;
    
    //Tween.go(&tweenVal, x, duration, 0, (EasingType)activeEaseType); // global/static
    
    multiTween.go(&multiVec.x, x, duration, 0, (EasingType)activeEaseType);
    multiTween.go(&multiVec.y, y, duration, 0, (EasingType)activeEaseType);
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    
    layer.onMoved(x, y);
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    
    // 2 second delay
    
    
    //ofLog() << tweenVal << ", " << x << ", " << duration << ", " << activeEaseType;
    //ofLog() << "1. adding new tween...";
    tween.go(&tweenVal, x, duration, 0, (EasingType)activeEaseType); // global/static
    
    //multiTween.go(&multiVec.x, x, duration, 0, (EasingType)activeEaseType);
    //int tweenId = multiTween.go(&multiVec.y, y, duration, 0, (EasingType)activeEaseType);
    //multiTween.onDone(tweenId, this, &testApp::onSomeTweenCallback); // call this directly after a tween.go as the tweenid changes.
    
    
    ofVec3f tweenToVec(x,y,0);
    //vecTween.go(&multiVec, tweenToVec, duration, 2, (EasingType)activeEaseType).onDone(this, &testApp::onSomeTweenCallback);
    //vecTween.onDone(tweenId, this, &testApp::onSomeTweenCallback); // call this directly after a tween.go as the tweenid changes.

    layer.onDown(x, y);
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

    //layer.setPosition(x, y);
    //layer.children[0]->setAlpha(ofRandom(255));
    //layer.removeLayer(0);
    
    layer.onUp(x, y);
    
    

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}





// arduino tips - http://cboard.cprogramming.com/c-programming/137150-floats-bytes-back-again.html
int breakDown(int index, unsigned char* outbox, float member){
    unsigned long d = *(unsigned long *)&member;    
    outbox[index] = d & 0x00FF;
    index++;    
    outbox[index] = (d & 0xFF00) >> 8;
    index++;    
    outbox[index] = (d & 0xFF0000) >> 16;
    index++;    
    outbox[index] = (d & 0xFF000000) >> 24;
    index++;
    return index;
}

float buildUp(int index, unsigned char* outbox){
    unsigned long d;    
    d =  (outbox[index+3] << 24) | (outbox[index+2] << 16)
    | (outbox[index+1] << 8) | (outbox[index]);
    float member = *(float *)&d;
    return member;
}


/// -------------- RANDOM TEST SHIT
void testApp::testBuffers() {
    
    //size_t sdSize = sizeof(SharedData);
    cout << sizeof(SharedData) << endl;
    //ofLog() << "sd: " << sdSize;
    
    SharedData sd;
    sd.x = 4321;
    sd.y = -9801;
    string heyStr = "hello folks";
    //strcpy(sd.str, heyStr.c_str());
    
    //ofLog() << sizeof(sd);
    cout << sizeof(sd) << endl;
    cout << sizeof(sd.x) << endl;
    cout << sizeof(sd.y) << endl;
    //ofLog() << sizeof(sd.x);
    //ofLog() << sizeof(sd.y);
    //ofLog() << sizeof(sd.str);
    
    char tester[11];
    //sprintf(tester, "%i %i %s", sd.x, sd.y, sd.str);
    sprintf(tester, "%d %d", sd.x, sd.y);
    /*sprintf(tester , "%i" , sd.x);
    sprintf(tester , "%i" , sd.y);
    sprintf(tester , "%s" , sd.str);*/
    //ofLog() << "ok? " << tester;
    
    //int a3[3] = {22445, 13, 1208132};
    int a3 = 22445; //127,87,0,0
    cout << "---" << sizeof(a3) << endl;
    
    unsigned char * c = (unsigned char *)&a3;
    cout << (unsigned int)c[0] << endl;
    cout << (unsigned int)c[1] << endl;
    
    cout << (unsigned int)c[2] << endl;
    cout << (unsigned int)c[3] << endl;
    cout << "---A" << endl;

    unsigned intA = (c[1] << 8) | (unsigned char)c[0];
    unsigned intB = (c[3] << 8) | (unsigned char)c[2];
    cout << intA << endl;
    cout << intB << endl;
    
    
    // convert int to char array, then back again. works for int only
    cout << "***" << endl;
    int aaa = -22445191; // 4 bytes
    char sInput[sizeof(aaa)];    
    sprintf(sInput , "%i" , aaa);
    cout << "\n*** A" << endl;
    int nn = atoi(sInput);
    cout << nn << endl;
    cout << sizeof(aaa) << endl;
    cout << "*** B\n" << endl;
    
    
    // safely add stuff to buffer- after x chars, buffer is cut off
    char buff[15];
    snprintf ( buff, 15, "The half xx of %d is %d 123456789101112", 60, 60/2 );
    cout << buff << endl;
    
    char bufff2[0]; // why is this working?
    sprintf(bufff2, " %f", -123456.1908); // 5 letters, ok
    cout << bufff2 << endl;
    printf("output :: |%s|\n" , bufff2);
    cout << atof(bufff2) << endl;
    
    
    float someNum = -2.765;
    unsigned char numBuff[1];
    int ind = breakDown(0, numBuff, someNum);
    for(int i=0; i<3; ++i) cout << numBuff[i] << endl;
    cout << ind << ", " << numBuff << endl;
    
    float val = buildUp(0,numBuff);
    cout << val << endl;
    //itoa (aaa,sInput,10);
    /*char *aaaa = (char *)&aaa;
    char *bee = (char *)&aaa;
    char b2 = *(bee+2); // = 87
    char b3 = *(bee+3); // = 173
    cout << b2 << endl;
    cout << b3 << endl;*/
    
    
    
    /*char cc[]={'1',':','3'};
    int ii = cc[0] - '0';
    //int ii=int(cc[0]);
    ofLog() << ii;    
    printf("%d",ii);*/
    
    
    
    
    
    /*uint8 *pBytes = (uint8 *)&aaa;
    pBytes[0] = (uint8)aaa;
    pBytes[1] = (uint8)(aaa >> 8);
    pBytes[2] = (uint8)(aaa >> 16);
    pBytes[3] = (uint8)(aaa >> 24);
    cout << pBytes[0] << endl;
    cout << pBytes[1] << endl;
    cout << pBytes[2] << endl;
    cout << pBytes[3] << endl;*/
    
    
    /*char b4[4];
    *(UInt32 *)b4 = htonl((UInt32)aaa);
    cout << *(b4+2) << endl;
    cout << *(b4+3) << endl;*/
    return;
    
    //http://stackoverflow.com/questions/6276576/how-to-convert-integer-to-char-array
    //http://stackoverflow.com/questions/8568450/integer-into-char-array?rq=1
    
     // -------
     
     
     double d = 984.1234;
     double *pd=&d;
     char *pc = reinterpret_cast<char*>(pd);
     //ofLog() << sizeof(double);
     for(size_t i=0; i<sizeof(double); i++)
     {
     char ch = *pc;
     //DoSomethingWith(ch);
     ofLog() << i << " : " << ch;
     pc++;
     }
     
     stringstream str;
     str << pc;
     ofLog() << "* double to char array: " << str.str();
     
     
     // --------------
     char chh[10]; //becuase double is 8 bytes in GCC compiler but take 10 for safety
     double ddd = -2415.23419;
     
     sprintf(chh , "%lf" , ddd);
     printf("output :: |%s|\n" , chh);
     return;
     // --------------
     
     // float buffer test...
     
     unsigned char charArray[100];
     float floatArray[100];
     stringstream ss;
     ss.precision(1.2);
     string delim = " ";
     // set a bunch of floats
     for(int i=0; i<10; ++i) {
     floatArray[i] = ofRandomWidth();
     ss << floatArray[i] << delim;
     
     ofLog() << floatArray[i];
     }
     ofLog() << "\n\n\n A--------";
     ofLog() << ss.str();
     //ss << kQuality << kDelimiter << quality;
     // set to chars
     for(int i=0; i<10; ++i) {
     charArray[i] = (char) floatArray[i];
     //ofLog() << charArray[i];
     }
     
     // sanity check
     ofLog() << "\n\n\n B--------";
     for(int i=0; i<10; ++i) {
     floatArray[i] = (float) charArray[i];
     //ofLog() << floatArray[i];
     }
     
     char buffer [50];
     int n, a=5, b=3;
     n=sprintf (buffer, "%d plus %d is %d", a, b, a+b);
     printf ("[%s] is a string %d chars long\n",buffer,n);
     
     
     // test 1
     
     int in = 22;
     float flt = 11.0;//.0;
     float dbl = 66.24;//.24;
     float lng = 29.5;//.5;
     int kDelim = 16; // 3 spaces between values
     int sizeAlloc = sizeof(int) + sizeof(float) + sizeof(float) + sizeof(float) + kDelim;// + 2; // 2 cents?
     ofLog() << sizeAlloc;
     ofLog() << sizeof(double);
     ofLog() << sizeof(float);
     sizeAlloc *= 20000;
     char charAr[sizeAlloc];//50*20000];
     float startTime = ofGetElapsedTimeMicros();
     int length = 0;
     for(int i = 0; i < 20000; i++) {
     length += sprintf(charAr+length, "%d %f %f %f", in, flt, dbl, lng);
     }
     //ofLog() << charAr;
     ofLog() << length;
     ofLog() << sizeAlloc;
     ofLog() << "time test 1 : " << (ofGetElapsedTimeMicros()-startTime);
     
     ofLog() << "\n\n";
     
     
     // test 2
     startTime = ofGetElapsedTimeMicros();
     stringstream sss;
     //sss.precision(10);
     for(int i = 0; i < 20000; i++) {
     sss << in << " " << flt << " " << dbl << " " << lng;
     if(i == 0) ofLog() << "Size: " << sss.str().size();
     }
     ofLog() << sss.str().size();
     ofLog() << "time test 2 : " << (ofGetElapsedTimeMicros()-startTime);
     

}



void testApp::drawTexturedTriangle(ofImage &img, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3){
    triPoints[0].set(x1,y1,z1);
    triPoints[1].set(x2,y2,z2);
    triPoints[2].set(x3,y3,z3);
    
    triTexPoints[0].set(0,0);
    triTexPoints[1].set(1,0);
    triTexPoints[2].set(1,1);
    ofSetColor(255);
    img.getTextureReference().bind();
    
    glEnableClientState(GL_TEXTURE_COORD_ARRAY_EXT);
    glTexCoordPointer(2, GL_FLOAT, sizeof(ofVec2f), &triTexPoints[0].x);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(ofVec3f), &triPoints[0].x);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    
    img.getTextureReference().unbind();
}