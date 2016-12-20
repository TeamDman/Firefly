#include "pins.h"
#include "domimg.h"

#include <HT1632.h>
#include <font_8x4.h>
#include <images.h>

#include <Wire.h>
#include <SparkFun_MMA8452Q.h>
MMA8452Q accel;

bool gameBlockData[6][12];
uint8_t gameBlockPaddle=6;
uint16_t gameBlockHeight=3;
uint8_t gameBlockPosition=0;

uint16_t gameGyroBall=0;

int i = 13; // start in middle
uint8_t wdImage;
uint8_t wdText;
uint8_t wd;
char disp [] = "suh dude";

bool doUpdate = true;
int16_t scollDelay = 0;
#define threshold 0.004

enum directions {
  none,
  forward,
  left,
  right,
  back
}   direction;

enum machineStates { 
  idle, 
  buttonPhysicalInput, 
  buttonLeftInput, 
  buttonRightInput, 
  busy, 
  sleeping
}   machineState;

enum scrollModes {
  scrollStop,
  scrollLeft,
  scrollRight
}   scrollMode;

enum displayModes {
  displayImage,
  displayText,
  displaySparkle,
  displayGyro,
  displayGameBlock,
  displayGameGyro
}   displayMode;

void buttonPhysical(void) {
  machineState = buttonPhysicalInput;
}
void buttonLeft(void) {
  machineState = buttonLeftInput;
}
void buttonRight(void) {
  machineState = buttonRightInput;
}


void setup () {
  machineState = idle;

  Serial.begin(115200);
  HT1632.begin(CS, SCK, MOSI);
  accel.init();

  pinMode (pushButton, INPUT_PULLUP);
  pinMode (capButtonRight, INPUT_PULLUP);
  pinMode (capButtonLeft, INPUT_PULLUP);

  attachInterrupt(pushButton, buttonPhysical, FALLING);
  attachInterrupt(capButtonLeft, buttonLeft, FALLING);
  attachInterrupt(capButtonRight, buttonRight, FALLING);

  Serial.println("\nFFv2\n");

  wdText = HT1632.getTextWidth(disp, FONT_8X4_END, FONT_8X4_HEIGHT);
  Serial.println(wd); // 46 for text
  wd = wdImage = 9; // for heart

}

void loop () {
  if (doUpdate) { 
    updateButtons();
    updateScrolling();
  }

  if (displayMode==displayGameBlock) {
    doModeGameBlock();
  } else if ( displayMode == displayGameGyro ) {
    doModeGameGyro();
  } else if ( displayMode == displaySparkle ) {
    doModeSparkle();
  } else if ( displayMode == displayGyro ) {
    doModeGyro();
  } else if ( displayMode == displayImage ) {
    doModeImage();
  } else if ( displayMode == displayText ) {
    doModeText();
  }

  HT1632.render();
  if (machineState == buttonPhysicalInput) {
    getNextMode();
  }
}

void getNextMode() {
  Serial.println("\nbutton physical\n");
  if ( displayMode == displayImage ) {
    displayMode = displayGameBlock;
    gameBlockReset();
    doUpdate=false;
  } else if (displayMode == displayGameBlock ) {
    displayMode = displayGameGyro;
    gameGyroReset();
  } else if ( displayMode == displayGameGyro) {
    displayMode = displaySparkle;
    wd = wdText;
  } else if ( displayMode == displaySparkle  ) {
    displayMode = displayGyro;
    doUpdate=true;
  } else if ( displayMode == displayGyro ) {
    displayMode = displayText;
  } else if ( displayMode == displayText ) {
    displayMode = displayImage;
    wd = wdImage;
  }

  i = 13;
  machineState = idle;
}

void updateButtons() {
  if (machineState == buttonLeftInput) {
    Serial.println("\nbutton left\n");
    scollDelay += 50;
    if (scollDelay > 200) {
      scollDelay = 200;
    }
    Serial.println(scollDelay);
    machineState = idle;
  }

  if (machineState == buttonRightInput) {
    Serial.println("\nbutton right\n");
    scollDelay -= 50;
    if (scollDelay < -200) {
      scollDelay = -200;
    }
    machineState = idle;
    Serial.println(scollDelay);
  }
}

void updateScrolling() {
  if ( scollDelay < 0 ) {
    scrollMode = scrollLeft;
  } else if ( scollDelay > 0 ) {
    scrollMode = scrollRight;
  } else if ( scollDelay == 0 ) {
    scrollMode = scrollStop;
  }

  if ( displayMode != displaySparkle) {
    delay(215 - abs(scollDelay)); // top speed is delay 15
  }

  if (scrollMode == scrollLeft) {
    if (i == 0) {
      i = wd + 16;
    }
    i--;
  }
  else if  (scrollMode == scrollRight) {
    if ( i == 16 + wd ) {
      i = 0;
    }
    i++; // = (i+1)%(wd + OUT_SIZE);
  }
}


void doModeSparkle() {
  HT1632.setPixelFF (random(16), random(12));
  HT1632.clearPixelFF (random(16), random(12));
  HT1632.clearPixelFF (random(16), random(12));
}

void doModeImage() {
  HT1632.clear();
  //HT1632.drawImageFF(IMG_HEART, IMG_HEART_WIDTH,  IMG_HEART_HEIGHT, 16 - i, 2);
  //HT1632.drawImageFF(IMG_CHECK, IMG_CHECK_WIDTH,  IMG_CHECK_HEIGHT, 16 - i, 2);
  HT1632.drawImageFF(IMG_BLOCK, IMG_BLOCK_WIDTH,  IMG_BLOCK_HEIGHT, 16 - i, 2);
}

void doModeText() {
  HT1632.clear();
  HT1632.drawTextFF(disp, 16 - i, 2, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
}



void gameBlockReset() {
  gameBlockPosition=0;
  gameBlockHeight=3;
  gameBlockPaddle=6;
  scrollMode=scrollRight;
  for (int x=0;x<6;x++) {
    for (int y=0;y<12;y++) {
      gameBlockData[x][y]=y<3;
    }
  }         
}

void doModeGameBlock() {
  delay(200 - gameBlockHeight*17);
  if (gameBlockPosition > 11 - gameBlockPaddle){
    scrollMode=scrollLeft;
  } else if (gameBlockPosition < 1){
    scrollMode=scrollRight;
  }
  if (machineState == buttonLeftInput || machineState==buttonRightInput) {
    uint8_t newPaddle = gameBlockPaddle;
    for (int x=0;x<gameBlockPaddle;x++) {
      if (x+gameBlockPosition-3<0 || x+gameBlockPosition-3>5 || !gameBlockData[x+gameBlockPosition-3][gameBlockHeight-1]) {
        newPaddle--;
      } else {
        gameBlockData[x+gameBlockPosition-3][gameBlockHeight]=true;
      }
    }
    gameBlockPaddle=newPaddle;
    gameBlockHeight++; 
    machineState=idle;
  }
  HT1632.clear();
  if (gameBlockPaddle<=0 || gameBlockHeight >= 12) {
    bool win = gameBlockPaddle > 0;
    for (int scroll=0;scroll<(win?48:42); scroll++) {
      HT1632.clear();
      HT1632.drawTextFF(win?"WINNER":"LOSER",16-scroll, 2, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
      HT1632.render();
      delay(50);
      if (machineState!=idle) {
        machineState=idle;
        break;
      }
    }
    gameBlockReset();
  } else {
    for (int x=0;x<6;x++) {
      for (int y=0;y<12;y++) {
        if (gameBlockData[x][y]){ 
          HT1632.setPixelFF(5+x,y);
        }
      }
      if (x<gameBlockPaddle){
        HT1632.setPixelFF(2+x+gameBlockPosition,gameBlockHeight);
      }
    }
    if (scrollMode == scrollRight) {
      gameBlockPosition++;
    } else if (scrollMode == scrollLeft) {
      gameBlockPosition--;
    }
  }
}

void setPixel(uint8_t x, uint8_t y, directions dir) {
  if (direction == dir) {
    HT1632.setPixelFF(x,y);
  }
}


void doModeGyro() {
  if (accel.available()) {
    accel.read();
    calcDirection();
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
  }
}


void gameGyroReset() {
  gameGyroBall=(7<<8)|6;
}
void doModeGameGyro() {
  if (accel.available()) {
    accel.read();
    calcDirection();
    HT1632.clear();

    uint8_t ballX = gameGyroBall>>8;
    uint8_t ballY = gameGyroBall&255;
    switch (direction) {
      case forward:
        ballY++;
        break;
      case back:
        ballY--;
        break;
      case left:
        ballX--;
        break;
      case right:
        ballX++;
        break;
    }
    if (ballX > 12)
      ballX=12;
    if (ballY > 12)
      ballY=12;
    if (ballX <0)
      ballX=0;
    if (ballY < 0)
      ballY=0;
    gameGyroBall=(ballX<<8)|ballY;
    HT1632.setPixelFF(ballX,ballY);
    delay(75);
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