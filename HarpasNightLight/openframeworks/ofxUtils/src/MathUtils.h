#pragma once

#include "ofMain.h"

// kinect for windows constants
#define NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS         (285.63f)   // Based on 320x240 pixel size.
#define NUI_CAMERA_DEPTH_NOMINAL_INVERSE_FOCAL_LENGTH_IN_PIXELS (3.501e-3f) // (1/NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS)
#define NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS         (531.15f)   // Based on 640x480 pixel size.
#define NUI_CAMERA_COLOR_NOMINAL_INVERSE_FOCAL_LENGTH_IN_PIXELS (1.83e-3f)  // (1/NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS)




// helper for double exponential smoothing
struct DoubleExpoData {
    
    ofVec3f target; // destination position
    ofVec3f smooth;
    ofVec3f doubleSmooth;
    ofVec3f predicted; // double expo smoothed + prediciton
};


class MathUtils {
public:
    
	// rotate to a direction params(a - b)
	static void rotateToNormal(ofVec3f normal) {
		normal.normalize();
	
		float rotationAmount;
		ofVec3f rotationAngle;
		ofQuaternion rotation;
	
		ofVec3f axis(0, -1, 0);
		rotation.makeRotate(axis, normal);
		rotation.getRotate(rotationAmount, rotationAngle);
		ofRotate(rotationAmount, rotationAngle.x, rotationAngle.y, rotationAngle.z);
	}

	// converts kinect real world coord to rgb/depth 640,480
	static ofVec3f kinectWorldToDepth(const ofVec3f& worldXYZ) {
		float x = ofGetWidth() / 2 + worldXYZ.x * (ofGetWidth() / 640.0f) * NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS / worldXYZ.z;
		float y = ofGetHeight() / 2 + worldXYZ.y * (ofGetHeight() / 480.0f) * NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS / worldXYZ.z;
		float z = worldXYZ.z; // fix this later...

		return ofVec3f(x,y,z);
	}
    
    // smooth vector of ofVec3f's from a -> b by ALPHA (0.01 = smooth, 1 = same)
	static void lowPassFilter(vector<ofVec3f> &smoothData, vector<ofVec3f> &data, float ALPHA = 0.25f) {

		//float ALPHA = 0.25f; //999f;
		for(int i = 0; i < smoothData.size(); i++) {
			//smoothData[i].x += (data[i].x - smoothData[i].x) * ALPHA;
			//smoothData[i].y += (data[i].y - smoothData[i].y) * ALPHA;
			//smoothData[i].z += (data[i].z - smoothData[i].z) * ALPHA;

			smoothData[i] += (data[i] - smoothData[i]) * ALPHA;
			//smoothData[i] = data[i] * (1 - ALPHA) + smoothData[i] * ALPHA;
		}
	}

	// smooth vector of ofVec3f's from a -> b by ALPHA (0.01 = smooth, 1 = same)
	// cutoff is the amount of movement limit before we shoudl reset
	static void lowPassFilterWithCutoff(vector<ofVec3f> &smoothData, vector<ofVec3f> &data, float ALPHA = 0.25f, float CUTOFF = 0.2f) {

		// check the topskull position as the single decider whether all points need to be reset.
		bool resetMovement = false;
		int trackedFacePoint = 0; // top skull
		if(data.size() > 0) {
			if(abs(data[trackedFacePoint].x - smoothData[trackedFacePoint].x) > CUTOFF || abs(data[trackedFacePoint].y - smoothData[trackedFacePoint].y) > CUTOFF || abs(data[trackedFacePoint].z - smoothData[trackedFacePoint].z) > CUTOFF) {
				ofLog() << abs(data[trackedFacePoint].z - smoothData[trackedFacePoint].z);
				resetMovement = true;
			}
		}
		

		//float ALPHA = 0.25f; //999f;
		for(int i = 0; i < smoothData.size(); i++) {
			//smoothData[i].x += (data[i].x - smoothData[i].x) * ALPHA;
			//smoothData[i].y += (data[i].y - smoothData[i].y) * ALPHA;
			//smoothData[i].z += (data[i].z - smoothData[i].z) * ALPHA;
			if(resetMovement) {
				smoothData[i] = data[i];
				/*smoothData[i].x = data[i].x;
				smoothData[i].y = data[i].y;
				smoothData[i].z = data[i].z;*/
			} else {
				smoothData[i] += (data[i] - smoothData[i]) * ALPHA;
			}
			
			//smoothData[i] = data[i] * (1 - ALPHA) + smoothData[i] * ALPHA;
		}
	}

	// http://blogs.interknowlogy.com/2011/10/27/kinect-joint-smoothing/
	// base value 0 - 1
	static float ExponentialMovingAverage( deque<float> &data, float ALPHA = 0.9f ) {
		float numerator= 0;
		float denominator = 0;

		int total = data.size();
		float sum = 0;
		for ( int i = 0; i < total; ++i )	{ 
			sum += data[i];
		}
		float average = sum / (float)total;
		
		for ( int j = 0; j < total; ++j )	{
			numerator += data[j] * pow( ALPHA, total - j - 1 );
			denominator += pow( ALPHA, total - j - 1 );
		}
		 
		numerator += average * pow( ALPHA, total );
		denominator += pow( ALPHA, total );
 
		return numerator / denominator;
	}
    
    
    /* 
        Updates a DoubleExpoData with all 4 values. Requires 3 arbitrary vectors to get the value: values.smooth, values.doubleSmooth, values.target. The value you want is values.predicted.
        - smoothAlpha 0 - 1 = no smoothing - very smooth
        - predictAlpha 0 - 1 = no prediction - heavy prediction/overshoot
     */
    static void DoubleExpoSmoothing(DoubleExpoData &values, float smoothAlpha = 0.75, float predictAlpha = 0.25) {
        
        // double smooth
        values.smooth = values.target * (1-smoothAlpha) + values.smooth * smoothAlpha;
        values.doubleSmooth = values.smooth * (1-smoothAlpha) + values.doubleSmooth * smoothAlpha;
        
        // prediction
        float ratio = predictAlpha / (1-predictAlpha); //(alpha*step) / (1-alpha);
        float a = 2 + ratio;
        float b = 1 + ratio;
        values.predicted = a * values.smooth - b * values.doubleSmooth;
    }
    
    /*
        Double expo smoothing- requires 4 ofVec3f's (target, smooth, doubleSmooth, and predicted)
     */
    static void DoubleExpoSmoothing(ofVec3f target, ofVec3f smooth, ofVec3f doubleSmooth, ofVec3f predicted, float smoothAlpha = 0.75, float predictAlpha = 0.25) {
        
        // double smooth
        smooth = target * (1-smoothAlpha) + smooth * smoothAlpha;
        doubleSmooth = smooth * (1-smoothAlpha) + doubleSmooth * smoothAlpha;
        
        // prediction
        float ratio = predictAlpha / (1-predictAlpha); //(alpha*step) / (1-alpha);
        float a = 2 + ratio;
        float b = 1 + ratio;
        predicted = a * smooth - b * doubleSmooth;
    }
    
    
    // http://www.flong.com/texts/code/shapers_poly/
    static float BlinnWyvillCosineApproximation (float x){
        float x2 = x*x;
        float x4 = x2*x2;
        float x6 = x4*x2;
        
        float fa = ( 4.0/9.0);
        float fb = (17.0/9.0);
        float fc = (22.0/9.0);
        
        float y = fa*x6 - fb*x4 + fc*x2;
        return y;
    }
    
    
    /*
        Polar to cartesian conversion (2D)
        - pass radius and angle and return a ofVec2f
     */
    static ofVec2f PolarToCartesian(float radius, float theta){
        float x = radius * cos(theta);
        float y = radius * sin(theta);
        
        return ofVec2f(x,y);
    }
    
    /*
        Cartesian to polar conversion (2D)
        - pass ofVec2f polar, and reference to a radius and angle
        - radius and theta are populated, void function return
     */
    static void CartesianToPolar(const ofVec2f &polar, float &radius, float &theta){
        radius = sqrt(polar.x * polar.x + polar.y * polar.y);
        theta = atan2(polar.y, polar.x);
    }
    
    /*
     Sphere to cartesian conversion (3D)
     - pass radius, polar angle (similar to latitude/rotation around horizontal axis), and azimuthal angle (similar to longitude/rotation around vertical axis) and return a ofVec3f
     */
    static ofVec3f SphereToCartesian(float radius, float theta, float phi){
        float x = cos(theta) * sin(phi) * radius;
        float y = sin(theta) * sin(phi) * radius;
        float z = cos(phi) * radius;
        
        return ofVec3f(x,y,z);
    }
    
    /*
     Cartesian to sphere conversion (3D)
     - pass ofVec2f pt, and reference to a radius, polar(theta) angle, and azimuthal(phi) angle
     - radius, theta, and phi are populated, void function return
     */
    static void CartesianToSphere(const ofVec3f &pt, float &radius, float &theta, float &phi){
        radius = sqrt(pt.x * pt.x + pt.y * pt.y + pt.z * pt.z);
        if (radius == 0) {
            theta = 0;
            phi = 0;
        } else {
            theta = acos(pt.z/radius);
            phi = atan2(pt.y, pt.x);
        }
    }
    
    /*
     Check if point is inside arbitrary shape
     - 2D hittest
     - T template shapePoints: vector<ofVec2f>, ofPolyline, etc
     - http://www.codeproject.com/Tips/84226/Is-a-Point-inside-a-Polygon
     */
    template <typename T>
    static bool isPointInsideShape(ofVec2f& pt, T& shapePoints) {
        
        int i, j = 0;
        bool c = false;
        int numPoints = shapePoints.size();
        for (i = 0, j = numPoints-1; i < numPoints; j = i++) {
            ofVec2f& v = shapePoints[i];
            ofVec2f& v2 = shapePoints[j];
            if ( ((v.y > pt.x) != (v2.y > pt.y)) && (pt.x < (v2.x - v.x) * (pt.y - v.y) / (v2.y - v.y) + v.x) ) {
                c = !c;
            }
        }
        return c;
    }
    
    
    /*
     Check if point is inside rect (x,y,width,height)
     - 2D hittest
     */
    static bool isPointInsideRect(ofVec2f& pt, int minX, int minY, int maxX, int maxY) {
        
        return pt.x > minX && pt.x < (minX+maxX) && pt.y > minY && pt.y < (minY+maxY);
    }
    
	static bool isPointInsideRect(const ofVec3f& pt, int minX, int minY, int maxX, int maxY) {
        
        return pt.x > minX && pt.x < (minX+maxX) && pt.y > minY && pt.y < (minY+maxY);
    }
    
    /*
     Check if point is inside quad (tl,tr,bl,br)
     - 2D hittest
     */
    static bool isPointInsideQuad(ofVec2f& pt, int minX, int minY, int maxX, int maxY) {
        
        return pt.x > minX && pt.x < maxX && pt.y > minY && pt.y < maxY;
        
    }
    
};

/*
 polyShape.addShapePoint(302, 409);
 polyShape.addShapePoint(281, 455);
 polyShape.addShapePoint(315, 468);
 polyShape.addShapePoint(236, 473);
 polyShape.addShapePoint(236, 438);
/*
// simple kalman filtering- http://stackoverflow.com/questions/15211435/smoothing-kinect-facetracking-data
double Q = 0.000001;
double R = 0.0001;
double P = 1, X = 0, K;

void resetKalman() {
	Q = 0.000001;
	R = 0.0001;
	P = 1, X = 0, K;
}

float SimpleKalman(float measurement)
{
	K = (P + Q) / (P + Q + R);
    P = R * (P + Q) / (R + P + Q);

    //measurementUpdate();
    double result = X + (measurement - X) * K;
    X = result;
    return result;
}

deque < ofMesh > dataValues; // temp store all data
void simpleKalmanFilter(ofMesh &mesh) {

	int storedDataEntries = 30;
	dataValues.push_back(mesh);
	if(dataValues.size() < storedDataEntries) {
		dataValues.pop_front();
	}
	//ofLog() << data.size();
	vector<ofVec3f>& data = mesh.getVertices();
	for(int i = 0; i < data.size(); i++) {
		resetKalman();
		float ax = 0;//data[i].x;
		for(int j = 0; j < dataValues.size(); j++) { 
			//vector<ofVec3f>& d = dataValues[j];
			vector<ofVec3f>& d = dataValues[j].getVertices();
			ax = SimpleKalman( d[i].x ); 
		}
		resetKalman();
		float ay = 0;//data[i].y;
		for(int j = 0; j < dataValues.size(); j++) { 
			//vector<ofVec3f>& d = dataValues[j];
			vector<ofVec3f>& d = dataValues[j].getVertices();
			ay = SimpleKalman( d[i].y ); 
		}
		resetKalman();
		float az = 0;//data[i].z;
		for(int j = 0; j < dataValues.size(); j++) { 
			//vector<ofVec3f>& d = dataValues[j];
			vector<ofVec3f>& d = dataValues[j].getVertices();
			az = SimpleKalman( d[i].z ); 
		}
		resetKalman();

		ofVec3f da(ax, ay, az);
		mesh.setVertex(i, da);
	}
}
// smooths all data values from a -> b

void simpleKalmanFilter(vector<ofVec3f> &prevData, vector<ofVec3f> &data) {

	
	for(int i = 0; i < prevData.size(); i++) {
		resetKalman();
		float ax = SimpleKalman( prevData[i].x );		
		float bx = SimpleKalman( data[i].x );
		resetKalman();
		float ay = SimpleKalman( prevData[i].y );		
		float by = SimpleKalman( data[i].y );
		resetKalman();
		float az = SimpleKalman( prevData[i].z );		
		float bz = SimpleKalman( data[i].z );
		resetKalman();

		data[i].set(bx, by, bz);
	}
}


void simpleKalmanFilter(ofVec3f* prevData, ofVec3f* data, int count) {
	for(int i = 0; i < count; i++) {
		resetKalman();
		float ax = SimpleKalman( prevData[i].x );		
		float bx = SimpleKalman( data[i].x );
		resetKalman();
		float ay = SimpleKalman( prevData[i].y );		
		float by = SimpleKalman( data[i].y );
		resetKalman();
		float az = SimpleKalman( prevData[i].z );		
		float bz = SimpleKalman( data[i].z );
		resetKalman();

		data[i].set(bx, by, bz);
	}
}*/