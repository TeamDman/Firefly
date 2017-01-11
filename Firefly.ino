#include "pins.h"
#include "domimg.h"

#include <HT1632.h>
#include <font_8x4.h>
#include <images.h>
#include <cmath>

#include <Wire.h>
#include <SparkFun_MMA8452Q.h>

enum directions {
  none,
  forward,
  left,
  right,
  back
}   direction, gameSnakeLast;

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
  displayGameSnake
}   displayMode;


MMA8452Q accel;

bool gameBlockData[6][12];
uint8_t gameBlockPaddle=6;
uint8_t gameBlockHeight=3;
uint8_t gameBlockPosition=0;

int8_t gameSnake[192][2];
int8_t gameSnakeOld[192][2];
uint8_t gameSnakeFruit[2];
uint8_t gameSnakeLength;

uint8_t i = 13; // start in middle
uint8_t wdImage;
uint8_t wdText;
uint8_t wd;
char disp [] = "DOMINIC PHILLIPS";

bool doUpdate = true;
int16_t scollDelay = 0;
#define threshold 0.01

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
  } else if ( displayMode == displayGameSnake ) {
    doModegameSnake();
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
    displayMode = displayGameSnake;
    gameSnakeReset();
  } else if ( displayMode == displayGameSnake) {
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
    for (uint8_t x=0;x<6;x++) {
      for (uint8_t y=0;y<12;y++) {
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


void gameSnakeReset() {
  gameSnakeGenFruit();
  for (uint8_t i=0;i<192;i++) {
    gameSnake[i][0]=-1;
    gameSnake[i][1]=-1;
  }
  gameSnake[0][0]=7;
  gameSnake[0][1]=6;
  gameSnakeLength=1;
  gameSnakeLast=none;
}

void gameSnakeGenFruit() {
  gameSnakeFruit[0]=random(1,14);
  gameSnakeFruit[1]=random(1,10);
}

void doModegameSnake() {
  if (accel.available()) {
    accel.read();
    calcDirection();

    if (direction == none)
      return;
    if ((int)direction==5-(int)gameSnakeLast)
      direction=gameSnakeLast;
    gameSnakeLast=direction;

    HT1632.clear();
    
    for (uint8_t i=0;i<gameSnakeLength;i++) {
      gameSnakeOld[i][0]=gameSnake[i][0];
      gameSnakeOld[i][1]=gameSnake[i][1];
    }

    switch (direction) {
      case forward:
        gameSnake[0][1]++;
        break;
      case back:
        gameSnake[0][1]--;
        break;
      case left:
        gameSnake[0][0]--;
        break;
      case right:
        gameSnake[0][0]++;
        break;
    }

    if (gameSnake[0][0] < 0 || gameSnake[0][1] < 0)
      gameSnakeReset();
    if (gameSnake[0][0] > 15 || gameSnake[0][1] > 11)
      gameSnakeReset();
    if (gameSnake[0][0] == gameSnakeFruit[0] && gameSnake[0][1] == gameSnakeFruit[1]) {
      gameSnakeGenFruit();
      gameSnakeLength++;
    }

    for (uint8_t i=1;i<gameSnakeLength;i++) {
      gameSnake[i][0] = gameSnakeOld[i-1][0];
      gameSnake[i][1] = gameSnakeOld[i-1][1];
    }
    for (uint8_t i=0;i<gameSnakeLength;i++) 
      HT1632.setPixelFF(gameSnake[i][0],gameSnake[i][1]);
    HT1632.setPixelFF(gameSnakeFruit[0],gameSnakeFruit[1]);
    delay(7000/fmax(abs(accel.cx*1000),abs(accel.cy*1000)));
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