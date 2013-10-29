
#pragma  once

#include "ofMain.h"

// globals
static GLdouble modelviewMatrix[16];
static GLdouble projectionMatrix[16];
static GLint viewport[4];


// static class
class MeshUtils{

public:
    
    
    // face/surface normal
    static ofVec3f getSurfaceNormal(ofVec3f v1, ofVec3f v2, ofVec3f v3) {
        ofVec3f u = v2-v1;
        ofVec3f v = v3-v1;
        ofVec3f n = u.crossed(v).normalized();
        return n;
    }
    
    /*
     For standard opengl triangles- GL_TRIANGLES
     - vertices must be ordered correctly (eg. anti clockwise)
     - indices are probably not set in this case or maybe must be sequential [0,1,2]- untested
     */
    static void calculateNormals(ofMesh &mesh) {
        
        vector<ofVec3f>& verts = mesh.getVertices();
        for(int i = 0; i < verts.size(); i+=3) {
            
            // 3 vertices make a triangle
            ofVec3f v1 = verts[i];
            ofVec3f v2 = verts[i+1];
            ofVec3f v3 = verts[i+2];
            
            // calc surface normalse
            ofVec3f n = getSurfaceNormal(v1, v2, v3);
            
            // add the same surface normal to all vertices (3)
            // flat shading?
            // may need to average instead?
            mesh.addNormal(n);
            mesh.addNormal(n);
            mesh.addNormal(n);
        }
    }
    
    /*
     Assumes indicies already set, most likely with a custom order
     - if indices are not set, just call mesh.setupIndicesAuto();
     */
    static void calculateIndexedNormals(ofMesh &mesh) {
        
        vector<ofVec3f>& verts = mesh.getVertices();
        vector<ofIndexType>& indices = mesh.getIndices();
        
        // set normals to default/zero - adds 1 normal per vertice
        ofVec3f nZero(0,0,0);
        mesh.clearNormals();
        for(int i = 0; i < verts.size(); i++) mesh.addNormal(nZero);
        vector<ofVec3f>& norms = mesh.getNormals();
        
        for(int i = 0; i < indices.size(); i+=3) {
            
            // indices/faces
            int index1 = indices[i];
            int index2 = indices[i+1];
            int index3 = indices[i+2];
            
            // 3 vertices make a triangle
            ofVec3f v1 = verts[index1];
            ofVec3f v2 = verts[index2];
            ofVec3f v3 = verts[index3];
            
            // calc surface normalse
            ofVec3f n = getSurfaceNormal(v1, v2, v3);
            
            // add to sum for averaging/normalise
            norms[index1] += n;
            norms[index2] += n;
            norms[index3] += n;
        }
        
        // normalise from average
        for(int i = 0; i < norms.size(); i++) norms[i].normalize();
    }
    
    
    
    /* 
     Camera <-> World transforms- thanks to Kyle McDonald
     - https://github.com/kylemcdonald/ofxFaceTracker/blob/master/example-blink/src/testApp.cpp
     - https://github.com/openframeworks/openFrameworks/issues/765
     */
    
    static void updateProjectionState() {
        glGetDoublev(GL_MODELVIEW_MATRIX, modelviewMatrix);
        glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
        glGetIntegerv(GL_VIEWPORT, viewport);
    }
    
    static ofVec3f ofWorldToScreen(ofVec3f world) {
        updateProjectionState();
        GLdouble x, y, z;
        gluProject(world.x, world.y, world.z, modelviewMatrix, projectionMatrix, viewport, &x, &y, &z);
        ofVec3f screen(x, y, z);
        screen.y = ofGetHeight() - screen.y;
        return screen;
    }
    
    
    /*
     Mesh transforms and tex coord helper
     */
    static ofMesh getProjectedMesh(const ofMesh& mesh) {
        ofMesh projected = mesh;
        for(int i = 0; i < mesh.getNumVertices(); i++) {
            ofVec3f cur = ofWorldToScreen(mesh.getVerticesPointer()[i]);
            cur.z = 0;
            projected.setVertex(i, cur);
        }
        return projected;
    }
    
    template <class T>
    static void addTexCoords(ofMesh& to, const vector<T>& from) {
        for(int i = 0; i < from.size(); i++) {
            to.addTexCoord(from[i]);
        }
    }
    
    
    /*template <typename T>
    static T getMeshPointsFromPath(T& points, float thickness) {
        T container;
    }*/
    /*
     Turns a path into a filled ofMesh. Same as ribbon code.
     - can pass in ofPolyLine, vector<ofPoints, deque<ofPoints>
     - ribbon is uniform in thickness. Use getMeshWithForceFromPath for variable thickness.
     */
    template <typename T>
    static ofMesh getMeshFromPath(T& points, float thickness) {
        ofMesh mesh;
        mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
        
        for(int i = 1; i < points.size(); i++){
            
            //find this point and the next point
            ofVec3f thisPoint = points[i-1];
            ofVec3f nextPoint = points[i];
            
            //get the direction from one to the next.
            //the ribbon should fan out from this direction
            ofVec3f direction = (nextPoint - thisPoint);
            
            //get the distance from one point to the next
            float distance = direction.length();
            
            //get the normalized direction. normalized vectors always have a length of one
            //and are really useful for representing directions as opposed to something with length
            ofVec3f unitDirection = direction.normalized();
            
            //find both directions to the left and to the right
            ofVec3f toTheLeft = unitDirection.getRotated(-90, ofVec3f(0,0,1));
            ofVec3f toTheRight = unitDirection.getRotated(90, ofVec3f(0,0,1));
            
            //calculate the points to the left and to the right
            //by extending the current point in the direction of left/right by the length
            ofVec3f leftPoint = thisPoint+toTheLeft * thickness;//otherThickness;
            ofVec3f rightPoint = thisPoint+toTheRight * thickness;//otherThickness;
            
            // fill mesh
            mesh.addVertex(leftPoint);
            mesh.addVertex(rightPoint);
            
            // draw the end? same orintation as previous
            if(i == points.size() - 1) {
                ofVec3f endLeftPoint = nextPoint+toTheLeft * thickness;//otherThickness;
                ofVec3f endRightPoint = nextPoint+toTheRight * thickness;//otherThickness;
                mesh.addVertex(endLeftPoint);
                mesh.addVertex(endRightPoint);
            }
        }
        
        return mesh;
    }
    
    /*
     Same as above but only draws half the mesh down the middle, either left side or right side
     */
    template <typename T>
    static ofMesh getSplitMeshFromPath(T& points, float thickness, bool drawLeftSide) {
        ofMesh mesh;
        mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
        
        for(int i = 1; i < points.size(); i++){
            
            //find this point and the next point
            ofVec3f thisPoint = points[i-1];
            ofVec3f nextPoint = points[i];
            
            //get the direction from one to the next.
            //the ribbon should fan out from this direction
            ofVec3f direction = (nextPoint - thisPoint);
            
            //get the distance from one point to the next
            float distance = direction.length();
            
            //get the normalized direction. normalized vectors always have a length of one
            //and are really useful for representing directions as opposed to something with length
            ofVec3f unitDirection = direction.normalized();
            
            //find both directions to the left and to the right
            ofVec3f toTheLeft = unitDirection.getRotated(-90, ofVec3f(0,0,1));
            ofVec3f toTheRight = unitDirection.getRotated(90, ofVec3f(0,0,1));
            
            //calculate the points to the left and to the right
            //by extending the current point in the direction of left/right by the length
            ofVec3f leftPoint, rightPoint;
            if(drawLeftSide) {
                leftPoint = thisPoint+toTheLeft;// * thickness;//otherThickness;
                rightPoint = thisPoint+toTheRight* thickness;//otherThickness;
            } else {
                leftPoint = thisPoint+toTheLeft * thickness;//otherThickness;
                rightPoint = thisPoint+toTheRight;//otherThickness;
            }
            
            
            // fill mesh
            mesh.addVertex(leftPoint);
            mesh.addVertex(rightPoint);
            
        }
        
        return mesh;
    }
    
    /*
     Thickness of mesh is affected by the distance between points.
     */
    template <typename T>
    static ofMesh getMeshWithForceFromPath(T& points, float thickness, float forceScale) {
        ofMesh mesh;
        mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
        
        for(int i = 1; i < points.size(); i++){
            
            //find this point and the next point
            ofVec3f thisPoint = points[i-1];
            ofVec3f nextPoint = points[i];
            
            //get the direction from one to the next.
            //the ribbon should fan out from this direction
            ofVec3f direction = (nextPoint - thisPoint);
            
            //get the distance from one point to the next
            float distance = direction.length();
            
            //get the normalized direction. normalized vectors always have a length of one
            //and are really useful for representing directions as opposed to something with length
            ofVec3f unitDirection = direction.normalized();
            
            //find both directions to the left and to the right
            ofVec3f toTheLeft = unitDirection.getRotated(-90, ofVec3f(0,0,1));
            ofVec3f toTheRight = unitDirection.getRotated(90, ofVec3f(0,0,1));
            
            float forceOffset = distance * forceScale;
            //calculate the points to the left and to the right
            //by extending the current point in the direction of left/right by the length
            ofVec3f leftPoint = thisPoint+toTheLeft * (thickness + forceOffset);//otherThickness;
            ofVec3f rightPoint = thisPoint+toTheRight * (thickness + forceOffset);//otherThickness;
            
            // fill mesh
            mesh.addVertex(leftPoint);
            mesh.addVertex(rightPoint);
            
            // draw the end? same orintation as previous
            /*if(i == points.size() - 1) {
             ofVec3f endLeftPoint = nextPoint+toTheLeft * (thickness + forceOffset);//otherThickness;
             ofVec3f endRightPoint = nextPoint+toTheRight * (thickness + forceOffset);//otherThickness;
             mesh.addVertex(endLeftPoint);
             mesh.addVertex(endRightPoint);
             }*/
        }
        
        return mesh;
    }
    
    template <typename T>
    static ofMesh getSplitMeshWithForceFromPath(T& points, float thickness, bool drawLeftSide, float forceScale) {
        ofMesh mesh;
        mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
        
        for(int i = 1; i < points.size(); i++){
            
            //find this point and the next point
            ofVec3f thisPoint = points[i-1];
            ofVec3f nextPoint = points[i];
            
            //get the direction from one to the next.
            //the ribbon should fan out from this direction
            ofVec3f direction = (nextPoint - thisPoint);
            
            //get the distance from one point to the next
            float distance = direction.length();
            
            //get the normalized direction. normalized vectors always have a length of one
            //and are really useful for representing directions as opposed to something with length
            ofVec3f unitDirection = direction.normalized();
            
            //find both directions to the left and to the right
            ofVec3f toTheLeft = unitDirection.getRotated(-90, ofVec3f(0,0,1));
            ofVec3f toTheRight = unitDirection.getRotated(90, ofVec3f(0,0,1));
            
            float forceOffset = distance * forceScale;
            //calculate the points to the left and to the right
            //by extending the current point in the direction of left/right by the length
            ofVec3f leftPoint, rightPoint;
            if(drawLeftSide) {
                leftPoint = thisPoint+toTheLeft;// * thickness;//otherThickness;
                rightPoint = thisPoint+toTheRight* (thickness + forceOffset);//otherThickness;
            } else {
                leftPoint = thisPoint+toTheLeft * (thickness + forceOffset);//otherThickness;
                rightPoint = thisPoint+toTheRight;//otherThickness;
            }
            
            
            // fill mesh
            mesh.addVertex(leftPoint);
            mesh.addVertex(rightPoint);
            
        }
        
        return mesh;
    }
    
    /*static ofMesh getMeshFromPath(ofPolyline& polyline, float thickness) {
        
        return getMeshFromPath(polyline.getVertices(), thickness);
    }*/
    
    template <typename T>
    static ofMesh getMeshFromPathLoop(T& points, float thickness) {
        ofMesh mesh;
        mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
        
        for(int i = 1; i < points.size(); i++){
            
            //find this point and the next point
            ofVec3f thisPoint = points[i-1];
            ofVec3f nextPoint = points[i];
            
            //get the direction from one to the next.
            //the ribbon should fan out from this direction
            ofVec3f direction = (nextPoint - thisPoint);
            
            //get the distance from one point to the next
            float distance = direction.length();
            
            //get the normalized direction. normalized vectors always have a length of one
            //and are really useful for representing directions as opposed to something with length
            ofVec3f unitDirection = direction.normalized();
            
            //find both directions to the left and to the right
            ofVec3f toTheLeft = unitDirection.getRotated(-90, ofVec3f(0,0,1));
            ofVec3f toTheRight = unitDirection.getRotated(90, ofVec3f(0,0,1));
            
            //calculate the points to the left and to the right
            //by extending the current point in the direction of left/right by the length
            ofVec3f leftPoint = thisPoint+toTheLeft * thickness;//otherThickness;
            ofVec3f rightPoint = thisPoint+toTheRight * thickness;//otherThickness;
            
            // fill mesh
            mesh.addVertex(leftPoint);
            mesh.addVertex(rightPoint);
            
            // draw the end? same orintation as previous
            if(i == points.size()-1) {
                ofVec3f endLeftPoint = points[1]+toTheLeft * thickness;//otherThickness;
                ofVec3f endRightPoint = points[1]+toTheRight * thickness;//otherThickness;
                mesh.addVertex(endLeftPoint);
                mesh.addVertex(endRightPoint);
            }
        }
        
        return mesh;
    }
};

