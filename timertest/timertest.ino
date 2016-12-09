/*
 Timer Interrupts Example

 Demonstrates usage of the HardwareTimer classes by blinking the LED
 
 Created 22 April 2010, last updated 8 June 2010
 By Bryan Newbold for LeafLabs
 This code is released with no strings attached.
 
 */

#include <boarddefs.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <ir_Lego_PF_BitStreamEncoder.h>

#include "pins.h"
#include "domimg.h"

#include <HT1632.h>
#include <font_8x4.h>
#include <images.h>



#define LED_RATE 50    // in microseconds

void handler_count1(void);
void handler_count2(void);

int pincheck = 0;
IRrecv irrecv(13);

void ButtonRight() {
    pincheck++;
}
void ButtonLeft() {
    pincheck--;
}

void setup()
{
	Serial.begin(115200); // Ignored by Maple. But needed by boards using hardware serial via a USB to Serial adaptor
    // Set up the LED to blink 
    pinMode(capButtonRight,INPUT_PULLUP);
    attachInterrupt(capButtonRight,ButtonRight,FALLING);
    pinMode(capButtonLeft,INPUT_PULLUP);
    attachInterrupt(capButtonLeft,ButtonLeft,FALLING);
    pinMode (pushButton, INPUT_PULLUP);
    HT1632.begin(CS, SCK, MOSI);
    irrecv.enableIRIn();
    // Setup Counting Timers
    Timer4.pause();
    Timer4.setPrescaleFactor(1);
    Timer4.setChannel1Mode(TIMER_OUTPUTCOMPARE);
    Timer4.setOverflow(2287);
    Timer4.setCompare1(300);
    Timer4.attachCompare1Interrupt(IRInterrupt);
    Timer4.setCompare2(200);
    Timer4.attachCompare2Interrupt(handler2);
    Timer4.resume();
}

void loop() {
    Serial.print("ir in ");
    Serial.print(pincheck);
    Serial.print(" ||| ");
    Serial.print(irparams.rcvstate);
    Serial.print(" ||| ");
    Serial.print(digitalRead(pincheck));//irparams.recvpin));


    if(digitalRead(pushButton)) {
        Serial.print(" pause");
        Timer4.pause();
    } else {
        Timer4.resume();
    }
    Serial.print("\n");
    delay(1);
} 
void handler2(void) {
    HT1632.setPixelFF (random(16), random(12));
    HT1632.clearPixelFF (random(16), random(12));
    HT1632.clearPixelFF (random(16), random(12));
    HT1632.render();
} 
