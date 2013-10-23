import joons.*;
import saito.objloader.*;

/* 
 Renders a 3d .obj file with the joons-renderer
 - use different key commands to render with different shaders 
 - renders single png image
 - renders multiple png's - auto rotates model
 */

// renderer and model
JoonsRenderer jr;
OBJModel model;
boolean rendered = false;
boolean showRender = true;
boolean renderMultiple = false;
int renderCount = 0;
boolean showText = true;

// render type
String renderLow = "ipr";
String renderHigh = "bucket";
String renderType = renderLow;

// custom shader types- see ambient.sc
String shaderRed = "Red2";
String shaderGreen = "Green";
String shaderBlue = "Blue";
String shaderPink = "Pink";
String shaderYellow = "Yellow";
String shaderOrange = "Orange";
String shaderCustom = "Custom";
String shaderMirror = "Mirror";
String shaderGlass = "Glass";
String shaderGlossy = "Glossy";
String shaderType = shaderCustom;

int shiftAmount = 25;
float weirdShiftAmount = PI / 16;
PVector eye = new PVector(0, 500, 625);//550, 1075);
PVector centre = new PVector(0, 150, 100);//0, 150);
PVector upAxis = new PVector(0, 0, -1);
PVector modelPos = new PVector(0, 0, 0);//250, 675);//0, 0);//180);
float floorZ = -220;
float fov = PI / 4; 
float aspect = (float) 1.3333;  //4/3
float zNear = 5;
float zFar = 10000;
float rx = 45, ry = 0, rz = 0; 
PVector[] positions = new PVector[100];
int[] sizes = new int[100];


public void setup() {
  size(1024, 768, P3D);

  textFont(createFont("Courier", 10));
  //textMode(SHAPE);

  jr = new JoonsRenderer(this, width, height);
  jr.setRenderSpeed(1);//render speed is 1 by default. Set it to 2 for x2 speed. Set it to any number. Lowers quality.
  /*jr.addBeforeShader("light {");
   jr.addBeforeShader("   type spherical");
   jr.addBeforeShader("   color { \"sRGB nonlinear\" 1.000 0.000 0.000 }");
   jr.addBeforeShader("   radiance 100.0");
   jr.addBeforeShader("   center 0 0 550");
   jr.addBeforeShader("   radius 50");
   jr.addBeforeShader("   samples 16");
   jr.addBeforeShader("}");
   jr.addBeforeShader("");*/

  model = new  OBJModel ( this, "elephanthead25-full.obj", "relative", TRIANGLES );
  model. scale (7);
}

public void draw() {

  background(255);
  pushMatrix();
  
  
  //lights(); // below is the same but brighter than 128
  int brightness = 255; // 128
  ambientLight(brightness,brightness, brightness);
  directionalLight(brightness, brightness, brightness, 0, 0, -1);
  lightFalloff(1, 0, 0);
  lightSpecular(0, 0, 0);

  beginRecord("joons.OBJWriter", "");//leave the second parameter as "".
  perspective(fov, aspect, zNear, zFar);//use perspective() before camera()!!
  camera(eye.x, eye.y, eye.z, centre.x, centre.y, centre.z, upAxis.x, upAxis.y, upAxis.z);

  // draw floor plane
  pushMatrix();
  translate(0, 0, floorZ);
  fill(130);//, 255, 0);
  //rect(-10000, -10000, 20000, 20000); //the floor plane
  beginShape(QUAD_STRIP); 
  vertex(-10000, -10000); 
  vertex(-10000, 20000);
  vertex(20000, -10000); 
  vertex(20000, 20000); 
  endShape();
  popMatrix();

  // draw model
  noSmooth(); //In my library, noSmooth() is used to separate one object from another.
  translate (modelPos.x, modelPos.y, modelPos.z);
  rotateX(rx);
  rotateY(ry);
  rotateZ(rz);
  fill(255);
  model.draw();

  endRecord();

  popMatrix();
  if (rendered && showRender)
    jr.display();

  // capture
  if (renderMultiple) {
    showText = false; // automatically switch this off when rendering multiple
    if (abs(rz)  < PI * 2 + weirdShiftAmount) {
      doRender();
      println("rendering frame... " + renderCount + ", " + rz);
      rz += weirdShiftAmount;
      renderCount++;
    } 
    else {
      println("*** finished render");
      rz = 0;
      renderCount = 0;
      renderMultiple = false;
      showText = true;
    }
  } 


  // draw text info
  if (showText) {
    
    String displayText = "3D MODEL / JOONS RENDERER / EXPORTER";
    displayText += "\n-";
    displayText += "\nCamEye XYZ (z/x,c/v,b/n) : " + modelPos.x + ", " + modelPos.y + ", " + modelPos.z;
    displayText += "\nCamCentre XYZ (t/g,y/h,u/j) : " + modelPos.x + ", " + modelPos.y + ", " + modelPos.z;
    displayText += "\nFloor Z ([]) : " + floorZ;
    displayText += "\n-";
    displayText += "\nModel XYZ (q/a,w/s,e/d) : " + modelPos.x + ", " + modelPos.y + ", " + modelPos.z;
    displayText += "\nModel rotateZ (,.) : " + rz;
    displayText += "\nModel rotateXY (mouseDragged) : " + rx + ", " + ry;
    displayText += "\n-";
    displayText += "\nRender type 'f' : " + renderType + ((renderType == renderLow) ? " (LQ fast)" : " (HQ slow)");
    displayText += "\nShader type (0-9) : " + shaderType;
    displayText += "\nHas rendered : " + rendered;
    displayText += "\nShow render 'm' : " + showRender;
    displayText += "\n-";
    displayText += "\nBegin multi render 'R'";
    displayText += "\nBegin single render 'r'";
    displayText += "\n-";
    displayText += "\nShow/hide text ' '";
    
    // reset camera perspective - don't know why push/pop doesnt work.
    float fov = PI/3.0;
    float cameraZ = (height/2.0) / tan(fov/2.0);
    perspective(fov, float(width)/float(height), 
                cameraZ/10.0, cameraZ*10.0);
    noLights();
    fill(100);
    text(displayText, 20, 20, 0);
  }
}


void doRender() {
  saveFrame("render/capture_" + nf(renderCount, 3) +".png");
  jr.setShader("object1", shaderType);
  jr.setSC("ambient.sc");
  rendered=jr.render(renderType);
}

public void keyPressed() {
  //if(rendered) return;
  if (key == 'r') {
    doRender();
  } 
  else if (key == 'R') {
    renderMultiple = !renderMultiple;
  } 
  else if(key == 'f') {
    renderType = (renderType == renderLow) ? renderHigh : renderLow;
  }
  
  else if (key == '1') {
    shaderType = shaderCustom;
  } 
  else if (key == '2') {
    shaderType = shaderRed;
  } 
  else if (key == '3') {
    shaderType = shaderBlue;
  } 
  else if (key == '4') {
    shaderType = shaderGlossy;//shaderGrey;
  } 
  else if (key == '5') {
    shaderType = shaderMirror;
  } 
  else if (key == '6') {
    shaderType = shaderGlass;
  } 
  else if (key == '7') {
    shaderType = shaderGreen;//shaderCustom;
  } 
  else if (key == '8') {
    shaderType = shaderYellow;
  } 
  else if (key == '9') {
    shaderType = shaderPink;
  } 
  else if (key == '0') {
    shaderType = shaderOrange;
  } 

  else if (key == 'q') {
    eye.x += shiftAmount;
  } 
  else if (key == 'a') {
    eye.x -= shiftAmount;
  } 
  else if (key == 'w') {
    eye.y += shiftAmount;
  } 
  else if (key == 's') {
    eye.y -= shiftAmount;
  } 
  else if (key == 'e') {
    eye.z += shiftAmount;
  } 
  else if (key == 'd') {
    eye.z -= shiftAmount;
  } 

  else if (key == 't') {
    centre.x += shiftAmount;
  } 
  else if (key == 'g') {
    centre.x -= shiftAmount;
  } 
  else if (key == 'y') {
    centre.y += shiftAmount;
  } 
  else if (key == 'h') {
    centre.y -= shiftAmount;
  } 
  else if (key == 'u') {
    centre.z += shiftAmount;
  } 
  else if (key == 'j') {
    centre.z -= shiftAmount;
  }

  else if (key == 'z') {
    modelPos.x += shiftAmount;
  } 
  else if (key == 'x') {
    modelPos.x -= shiftAmount;
  } 
  else if (key == 'c') {
    modelPos.y += shiftAmount;
  } 
  else if (key == 'v') {
    modelPos.y -= shiftAmount;
  } 
  else if (key == 'b') {
    modelPos.z += shiftAmount;
  } 
  else if (key == 'n') {
    modelPos.z -= shiftAmount;
  }

  else if (key == '[') {
    floorZ += shiftAmount;
  } 
  else if (key == ']') {
    floorZ -= shiftAmount;
  } 

  else if (key == 'm') {
    showRender = !showRender;
  }

  else if (key == ',') {
    rz += weirdShiftAmount;
    //renderCount++;
  } 
  else if (key == '.') {
    rz -= weirdShiftAmount;
  }

  else if(key == ' ') {
    showText = !showText;
  }

  println("eyex: " + eye.x + ", eyey: " + eye.y + ", eyez: " + eye.z);
  println("cx: " + centre.x + ", cy: " + centre.y + ", cz: " + centre.z);
  println("modelx: " + modelPos.x + ", modely: " + modelPos.y + ", modelz: " + modelPos.z);
}

void mouseDragged() {
  rx += (pmouseY-mouseY)*.01;
  ry += (pmouseX-mouseX)*-.01;
}
