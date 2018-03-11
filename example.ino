#include "Arduino.h"

#include "Receiver.cpp"
#include "Commands.cpp"
#include "Sender.cpp"

#define IR_RECEIVE_PIN 6  
#define IR_SEND_PWM_PIN 11 // has to be PWM pin

Receiver receiver(IR_RECEIVE_PIN);
ISR(TIMER1_OVF_vect) {
    receiver.tick();
}   // interrupt service routine

Sender sender(IR_SEND_PWM_PIN);

void setup() {
    Serial.begin(9600);
    Serial.println("Daikin remote demo");
    receiver.begin();
}

void loop() {
    // uncomment to see demo

    //demoSending();
    //demoReceiving();
}

void demoSending() {
    Serial.println("Demo sending");

    sendWithDelay(CMD_ONOFF);

    sendWithDelay(CMD_FAN);
    sendWithDelay(CMD_FAN);
    sendWithDelay(CMD_FAN);
    sendWithDelay(CMD_FAN);

    sendWithDelay(CMD_BRIGH);
    sendWithDelay(CMD_BRIGH);
    sendWithDelay(CMD_BRIGH);

    sendWithDelay(CMD_TIMER);
    sendWithDelay(CMD_TIMER);
    sendWithDelay(CMD_TIMER);

    sendWithDelay(CMD_ANTI);
    sendWithDelay(CMD_AUTO);
    sendWithDelay(CMD_SLEEP);
    sendWithDelay(CMD_TURBO);

    //sendWithDelay(CMD_LOCK);
    //sendWithDelay(CMD_LOCK);

    sendWithDelay(CMD_ONOFF);

}

void sendWithDelay(const byte *body) {
    sender.sendCommand(body);
    delay(500);
}

void demoReceiving() {    

    if (receiver.hasData()) {
		Serial.println("Demo receiving");
        Receiver::eButton btn = receiver.getButton();
        Serial.print("button ");
        String btnStr = receiver.getButtonStr(btn);
        Serial.println(btnStr);
        receiver.reset();
    }

    if (receiver.isError())
        receiver.reset();
}
