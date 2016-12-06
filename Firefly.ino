
//  NEW COMMANDS

//  HT1632.setPixelFF(x,y);  - remapped pixel coordinates, origin (0,0) at bottom left, x is horizontal, y is vertical - requires library update
//  HT1632.drawTextFF(disp, OUT_SIZE - i, 2, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT); - works same as prev version but is rotated and remapped
// fix timing problem
// 38 kHz wave
#include "pins.h"
#include "domimg.h"

#include <HT1632.h>
#include <font_8x4.h>
#include <images.h>

#include <HardWire.h>


HardWire HWire(1, I2C_FAST_MODE); // I2c1
uint8_t accData[64];

bool blockGameData[6][12];
uint8_t blockGamePaddle=6;
uint16_t blockGameHeight=3;
uint8_t blockGamePosition=0;


int i = 13; // start in middle
uint8_t wdImage;
uint8_t wdText;
uint8_t wd;
char disp [] = "HARAMBE WAS JUST A GORILLA";

int16_t scollDelay = 0;

enum machineStates { idle, buttonPhysicalInput, buttonLeftInput, buttonRightInput, busy, sleeping } machineState;
enum scrollModes { scrollStop, scrollLeft, scrollRight } scrollMode;
enum displayModes { displayImage, displayText, displaySparkle, displayBlockGame } displayMode;

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
  HWire.begin();

  pinMode (pushButton, INPUT_PULLUP);
  pinMode (capButtonRight, INPUT_PULLUP);
  pinMode (capButtonLeft, INPUT_PULLUP);

  attachInterrupt(pushButton, buttonPhysical, FALLING);
  attachInterrupt(capButtonLeft, buttonLeft, FALLING);
  attachInterrupt(capButtonRight, buttonRight, FALLING);

  delay (2000);
  Serial.println("\nFFv2\n");

  wdText = HT1632.getTextWidth(disp, FONT_8X4_END, FONT_8X4_HEIGHT);
  Serial.println(wd); // 46 for text
  wd = wdImage = 9; // for heart


 // HWire.requestFrom (0x1C, 5);
}



void readi2c (void) {

  HWire.requestFrom (0x1C, 32);
  for (int i = 0; i < 32; i++) {
    accData[i] = HWire.read();
  }
  //HWire.endTransmission ();

  HWire.requestFrom (0x1C, 32);
  for (int i = 33; i < 64; i++) {
    accData[i] = HWire.read();
  }
  //HWire.endTransmission ();

  for (int i = 0; i < 63; i++) {
    Serial.print ("reg ");
    Serial.print(i);
    Serial.print (" data ");
    Serial.println(accData[i]);

  }
}
  
void blockGameReset() {
  displayMode = displayBlockGame ;
  blockGamePosition=0;
  blockGameHeight=3;
  blockGamePaddle=6;
  scrollMode=scrollRight;
  for (int x=0;x<6;x++) {
    for (int y=0;y<12;y++) {
      blockGameData[x][y]=y<3;
    }
  }         
}

void loop () {
  if (displayMode==displayBlockGame) {
    delay(200 - blockGameHeight*17);
    if (blockGamePosition > 11 - blockGamePaddle){
      scrollMode=scrollLeft;
    } else if (blockGamePosition < 1){
      scrollMode=scrollRight;
    }
    if (machineState == buttonLeftInput || machineState==buttonRightInput) {
      uint8_t newPaddle = blockGamePaddle;
      for (int x=0;x<blockGamePaddle;x++) {
        if (x+blockGamePosition-3<0 || x+blockGamePosition-3>5 || !blockGameData[x+blockGamePosition-3][blockGameHeight-1]) {
          newPaddle--;
        } else {
          blockGameData[x+blockGamePosition-3][blockGameHeight]=true;
        }
      }
      blockGamePaddle=newPaddle;
      blockGameHeight++; 
      machineState=idle;
    }
    HT1632.clear();
    if (blockGamePaddle<=0 || blockGameHeight >= 12) {
      bool win = blockGamePaddle > 0;
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
      blockGameReset();
    } else {
      for (int x=0;x<6;x++) {
        for (int y=0;y<12;y++) {
          if (blockGameData[x][y]){ 
            HT1632.setPixelFF(5+x,y);
          }
        }
        if (x<blockGamePaddle){
          HT1632.setPixelFF(2+x+blockGamePosition,blockGameHeight);
        }
      }
      if (scrollMode == scrollRight) {
        blockGamePosition++;
      } else if (scrollMode == scrollLeft) {
        blockGamePosition--;
      }
    }
  }

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

  if (machineState == buttonPhysicalInput) {
    Serial.println("\nbutton physical\n");
    
    if ( displayMode == displayImage ) {
      blockGameReset();
      wd = wdText;
    }
    else if ( displayMode == displayText ) {
      displayMode = displayImage ;
      wd = wdImage;
    }
    else if ( displayMode == displaySparkle  ) {
      displayMode = displayText ;
    } else if (displayMode == displayBlockGame ) {
      displayMode = displaySparkle;
    }


    i = 13; // reset pointer to manage out of bounds on text -> image
    //scrollMode = scrollStop;
    //scollDelay = 0;
    machineState = idle;
  }


  if ( displayMode == displaySparkle ) {
    HT1632.setPixelFF (random(16), random(12));
    HT1632.clearPixelFF (random(16), random(12));
    HT1632.clearPixelFF (random(16), random(12));
    //delay(5);
  }
  else if ( displayMode == displayImage ) {
    HT1632.clear();
    //HT1632.drawImageFF(IMG_HEART, IMG_HEART_WIDTH,  IMG_HEART_HEIGHT, 16 - i, 2);
    // HT1632.drawImageFF(IMG_CHECK, IMG_CHECK_WIDTH,  IMG_CHECK_HEIGHT, 16 - i, 2);
    HT1632.drawImageFF(IMG_BLOCK, IMG_BLOCK_WIDTH,  IMG_BLOCK_HEIGHT, 16 - i, 2);
  }
  else if ( displayMode == displayText ) {
    HT1632.clear();
    HT1632.drawTextFF(disp, 16 - i, 2, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
  }
  HT1632.render(); // This updates the display on the screen.


  if (displayMode != displayBlockGame) { 
    if ( scollDelay < 0 ) {
      scrollMode = scrollLeft;
    }
    else if ( scollDelay > 0 ) {
      scrollMode = scrollRight;
    }
    else if ( scollDelay == 0 ) {
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
    else {
      ;  // stop
    }
  }


}
