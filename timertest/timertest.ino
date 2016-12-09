
#include "pins.h"

int pincheck = 0;

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

}

void loop() {
    Serial.print("ir in ");
    Serial.print(pincheck);
    Serial.print(" ||| ");
    Serial.print(digitalRead(pincheck));//irparams.recvpin));

    Serial.print("\n");
} 