#include <IRremote.h>
#include <IRremoteInt.h>

#include "pins.h"
#include "domimg.h"

#include <HT1632.h>
#include <font_8x4.h>
#include <images.h>



#define LED_RATE 50    // in microseconds

void handler_count1(void);
void handler_count2(void);

int pincheck = 0;
IRrecv irrecv(3);
decode_results results;

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
}

void loop() {
// STATE_IDLE      2
// STATE_MARK      3
// STATE_SPACE     4
// STATE_STOP      5
// STATE_OVERFLOW  6
    // Serial.print("ir in ");
    // Serial.print(irparams.recvpin);
    // Serial.print(" ||| ");
    // Serial.print(digitalRead(irparams.recvpin));
    // Serial.print(" ||| ");
    // Serial.print(irparams.rcvstate);
    // Serial.print(" ||| ");
    if (irrecv.decode(&results)) {
        Serial.println(results.value);
        irrecv.resume();
    }

    if(digitalRead(pushButton)) {
        Timer4.pause();
    } else {
        Timer4.resume();
    }
    delay(1);
} 
void handler2(void) {
    HT1632.setPixelFF (random(16), random(12));
    HT1632.clearPixelFF (random(16), random(12));
    HT1632.clearPixelFF (random(16), random(12));
    HT1632.render();
} 
