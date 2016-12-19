#include <Wire.h> // Must include Wire library for I2C
#include <SparkFun_MMA8452Q.h> // Includes the SFE_MMA8452Q library



#include "pins.h"
#include <HT1632.h>
#include <font_8x4.h>
#include <images.h>

MMA8452Q accel;

#define threshold 0.02

enum directions { none, forward, left, right, back } direction;
char dir = 0x0000;

void setup()
{
  Serial.begin(9600);  
  // Choose your adventure! There are a few options when it comes
  // to initializing the MMA8452Q:
  //  1. Default init. This will set the accelerometer up
  //     with a full-scale range of +/-2g, and an output data rate
  //     of 800 Hz (fastest).
  accel.init();
  HT1632.begin(CS, SCK, MOSI);
  direction = none;
  //  2. Initialize with FULL-SCALE setting. You can set the scale
  //     using either SCALE_2G, SCALE_4G, or SCALE_8G as the value.
  //     That'll set the scale to +/-2g, 4g, or 8g respectively.
  //accel.init(SCALE_4G); // Uncomment this out if you'd like
  //  3. Initialize with FULL-SCALE and DATA RATE setting. If you
  //     want control over how fast your accelerometer produces
  //     data use one of the following options in the second param:
  //     ODR_800, ODR_400, ODR_200, ODR_100, ODR_50, ODR_12,
  //     ODR_6, or ODR_1. 
  //     Sets to 800, 400, 200, 100, 50, 12.5, 6.25, or 1.56 Hz.
  //accel.init(SCALE_8G, ODR_6);
}

void loop()
{
  if (accel.available())
  {
    accel.read();
    
    calcDirection();
    Serial.print(direction);
    Serial.print("\t");
    Serial.print(dir);
    Serial.print("\t");
    blinkAccels();

    printCalculatedAccels();
    Serial.print("\t");
    printAccels(); 
    
    printOrientation();
      
    
    Serial.println(); 
  }
}

void blinkAccels() {
  HT1632.clear();
  HT1632.setPixelFF(2,11);

  HT1632.setPixelFF(8+accel.cx*7,8);
  HT1632.setPixelFF(7+accel.cx*7,8);
  HT1632.setPixelFF(8+accel.cy*7,6);
  HT1632.setPixelFF(7+accel.cy*7,6);
  HT1632.setPixelFF(8+accel.cz*7,4);
  HT1632.setPixelFF(7+accel.cz*7,4);

  HT1632.setPixelFF(2,1);
  setPixel(2,2,forward);
  setPixel(2,0,back);
  setPixel(3,1,right);
  setPixel(1,1,left);

  HT1632.render();
}
void setPixel(uint8_t x, uint8_t y, directions dir) {
  if (direction == dir) {
    HT1632.setPixelFF(x,y);
  }
}

void calcDirection() {
  direction=none;
  if (accel.cx > threshold && accel.cy > threshold) {
    direction = forward;
  } else if (accel.cx < -threshold && accel.cy < -threshold) {
    direction = back;
  } else if (accel.cx > threshold && accel.cy < -threshold) {
    direction = right;
  } else if (accel.cx < -threshold && accel.cy > threshold) {
    direction = left;
  }
}

void printAccels()
{
  Serial.print(accel.x, 3);
  Serial.print("\t");
  Serial.print(accel.y, 3);
  Serial.print("\t");
  Serial.print(accel.z, 3);
  Serial.print("\t");
}

void printCalculatedAccels()
{ 
  Serial.print(accel.cx, 3);
  Serial.print("\t");
  Serial.print(accel.cy, 3);
  Serial.print("\t");
  Serial.print(accel.cz, 3);
  Serial.print("\t");
}

void printOrientation()
{
  byte pl = accel.readPL();
  switch (pl)
  {
  case PORTRAIT_U:
    Serial.print("Portrait Up");
    break;
  case PORTRAIT_D:
    Serial.print("Portrait Down");
    break;
  case LANDSCAPE_R:
    Serial.print("Landscape Right");
    break;
  case LANDSCAPE_L:
    Serial.print("Landscape Left");
    break;
  case LOCKOUT:
    Serial.print("Flat");
    break;
  }
}
