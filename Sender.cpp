#ifndef __DAIKIN_SENDER_CLASS
#define __DAIKIN_SENDER_CLASS

#include <Arduino.h>
#include "Commands.cpp"

class Sender {
    const int IR_SEND_PIN;

private:

    void enableIROut() {
        TIMSK2 = 0;
        TCCR2A = _BV(WGM21) | _BV(COM2A0);  //18.11.1 TCCR2A – Timer/Counter Control Register A
        TCCR2B = _BV(CS20);                //18.11.2 TCCR2B – Timer/Counter Control Register B
        OCR2A = (F_CPU / (36000L * 2L) - 1);    //36000L - 36kHz
    }

    void mark(const unsigned int time) {
        pinMode(IR_SEND_PIN, OUTPUT);
        delayMicroseconds(time);
    }

    void space(const unsigned int time) {
        pinMode(IR_SEND_PIN, INPUT);
        delayMicroseconds(time);
    }

    void send(const bool *buff) {
        mark(3600);
        space(1600);

        for (int i = 0; i < 64; i++) {
            mark(400);
            space((buff[i] == true) ? 1300 : 500);
        }

        mark(400);
        space(0);
    }

    void generateMessages(bool * ret, const byte *header, const byte * body) {
        int retI = 0;

        for (int i = 0; i < 3; i++)
            for (int b = 7; b >= 0; b--)
                ret[retI++] = bitRead(PREFIX[i], b);
        for (int i = 0; i < 2; i++)
            for (int b = 7; b >= 0; b--)
                ret[retI++] = bitRead(header[i], b);
        for (int i = 0; i < 3; i++)
            for (int b = 7; b >= 0; b--)
                ret[retI++] = bitRead(body[i], b);
    }

public:

    Sender(int outPin) :
            IR_SEND_PIN(outPin) {
        pinMode(IR_SEND_PIN, INPUT);    //output dissabled
        digitalWrite(IR_SEND_PIN, LOW);
    }

    void sendCommand(const byte *cmdBody) {
        bool message1[64];
        generateMessages(message1, HEADER_BEGIN, HEADER_BODY);

        bool message2[64];
        generateMessages(message2, CMD_BEGIN, cmdBody);

        enableIROut();

        //header
        send(message1);

        //wait less than 30000 usec
        delayMicroseconds(10000);
        delayMicroseconds(10000);
        delayMicroseconds(9800);
        //--

        //payload
        send(message2);

        //create a 30000 usec delay before sending next command
        delayMicroseconds(10000);
        delayMicroseconds(10000);
        delayMicroseconds(10000);
        //--
    }
};

#endif // __DAIKIN_SENDER_CLASS
