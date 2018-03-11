#ifndef __DAIKIN_RECEIVER_CLASS
#define __DAIKIN_RECEIVER_CLASS

#include <Arduino.h>
#include "Commands.cpp"

/*
 * We want to measure gaps between changing signals that are approximately ~300us and longer
 * so we need a timer to fire around that frequent - not less than once every 300us
 * but also not too often (too many calls will drain all our cpu processing power)
 * Best value seems to be 100us interval
 *
 * 16Mhz clock ticks every -> 62,5ns
 * with prescaler x8 every timer tick that will be 62,5 x 8 = 500ns = 0.5us
 * to get 100us we need 100us/0.5us = 200 timer ticks
 *
 * thus, the timer params:
 * prescaler = x8: TCCR1B CS12 = 0, CS11 = 1, CS10 = 0
 * timer counting to 200: TCNT1 = MAX_UNSIGNED_INT - 200
 *
 * 200 ticks = 100us
 * 100 ticks =  50us
 *  50 ticks =  25us
 */

#define MAX_UNSIGNED_INT 65535
#define TIMER_TICKS 200
#define LAST_TIMEOUT 31000 // us

#define SPACE_LONG     3500
#define SPACE           435
#define MARK_START     1760
#define MARK_0          435
#define MARK_1         1295
#define MARK_TIMEOUT  30000
#define OFFSET          200

class Receiver {

    const unsigned int TIMER_PRELOAD = MAX_UNSIGNED_INT - TIMER_TICKS;
    const int IR_RECEIVE_PIN;

    Stream* debug = 0;

    bool debugEnabled = false;

    unsigned long lastActionTime = 0;  //micros
    bool lastInput = HIGH;

    byte timeouts = 0;

    byte data[16];
    byte index = 0;

    enum eReceiverState {
        STOP, IDLE, RUN, TIMEOUT, OVERFLOW
    } receiverState = STOP;

    enum eReadState {
        NONE, READ_START, READ_SPACE, READ_MARK, READ_TIMEOUT, ERROR
    } readState = NONE;

public:

    enum eButton {
        ONOFF, FAN, ANTI, SLEEP, BRIGH, TIMER, TURBO, AUTO, LOCK, unknown
    };

    Receiver(const int irReceivePin) :
            IR_RECEIVE_PIN(irReceivePin) {
    }

    Receiver(const int irReceivePin, Stream *_debug) :
            IR_RECEIVE_PIN(irReceivePin) {
        debug = _debug;
        debugEnabled = true;
    }

    void begin() {
        pinMode(IR_RECEIVE_PIN, INPUT);

        noInterrupts();
        {
            TCCR1A = 0;
            TCCR1B = 0;
            TCNT1 = TIMER_PRELOAD;

            // enable timer overflow interrupt
            TIMSK1 |= (1 << TOIE1);

            // set x8 prescaler
            bitSet(TCCR1B, CS11);

        }
        interrupts();

        reset();
    }

    void reset() {
        lastInput = HIGH;
        lastActionTime = 0;
        index = 0;
        receiverState = IDLE;
        readState = NONE;
        timeouts = 0;
        for (int i = 0; i < 16; i++) {
            data[i] = 0;
        }
    }

    void tick() {
        TCNT1 = TIMER_PRELOAD;
        bool input = digitalRead(IR_RECEIVE_PIN);

        switch (receiverState) {

        case IDLE: {
            if (input == LOW) {
                receiverState = RUN;
                lastActionTime = micros();
                lastInput = LOW;
            }
            break;
        }

        case RUN: {
            onTick(lastInput != input);
            lastInput = input;
            break;
        }

        case STOP:
        case OVERFLOW:
        case TIMEOUT:
            return;
        }
    }

    void onTick(bool newAction) {

        unsigned long timeDelta = abs(micros() - lastActionTime);

        if (index >= 128) {
            receiverState = OVERFLOW;
            return;
        }

        if (timeDelta > LAST_TIMEOUT) {
            receiverState = TIMEOUT;
            return;
        }

        if (newAction) {
            lastActionTime = micros();

            switch (readState) {
            case NONE:
                if (isEqual(timeDelta, SPACE_LONG))
                    readState = READ_START;
                else
                    readState = ERROR;
                break;

            case READ_START:
                if (isEqual(timeDelta, MARK_START))
                    readState = READ_SPACE;
                else
                    readState = ERROR;
                break;

            case READ_SPACE:
                if (isEqual(timeDelta, SPACE))
                    readState = READ_MARK;
                else
                    readState = ERROR;
                break;

            case READ_MARK:
                if (isEqual(timeDelta, MARK_0)) {
                    bitClear(data[index / 8], 7 - index % 8);
                    readState = READ_SPACE;
                    index++;
                } else if (isEqual(timeDelta, MARK_1)) {
                    bitSet(data[index / 8], 7 - index % 8);
                    readState = READ_SPACE;
                    index++;
                } else if (isEqual(timeDelta, MARK_TIMEOUT)) {
                    readState = READ_TIMEOUT;
                    timeouts++;
                } else {
                    readState = ERROR;
                }
                break;

            case READ_TIMEOUT:
                if (timeouts == 1) {
                    if (isEqual(timeDelta, SPACE_LONG))
                        readState = READ_START;
                    else
                        readState = ERROR;
                } else if (timeouts == 2) {
                }
                break;
            }

            if (readState == ERROR) {
                receiverState = STOP;
            }

            return;
        }

    }

    bool hasData() {
        return (receiverState == TIMEOUT || receiverState == OVERFLOW) && index == 128;
    }

    bool isError() {
        return readState == ERROR || receiverState == STOP;
    }

    eButton getButton() {

        bool isOK = true;
        eButton button = unknown;

        //check len
        if (index != 128) {
            print("error len");
            print(index);
            print(" expected 128");
            return unknown;
        }

        byte *part1 = &data[0];
        byte *part2 = &data[8];

        if (!compare(part1, PREFIX, 0, 3)) {
            print("error: header-prefix");
        }
        if (!compare(part1, HEADER_BEGIN, 3, 2)) {
            print("error: header-begin");
        }
        if (!compare(part1, HEADER_BODY, 3 + 2, 3)) {
            print("error: header-body");
        }

        if (!compare(part2, PREFIX, 0, 3)) {
            print("error: cmd-prefix");
        }
        if (!compare(part2, CMD_BEGIN, 3, 2)) {
            print("error: cmd-begin");
        }

        if (compare(part2, CMD_BRIGH, 5, 3)) {
            button = BRIGH;
        } else if (compare(part2, CMD_TIMER, 5, 3)) {
            button = TIMER;
        } else if (compare(part2, CMD_SLEEP, 5, 3)) {
            button = SLEEP;
        } else if (compare(part2, CMD_ANTI, 5, 3)) {
            button = ANTI;
        } else if (compare(part2, CMD_TURBO, 5, 3)) {
            button = TURBO;
        } else if (compare(part2, CMD_FAN, 5, 3)) {
            button = FAN;
        } else if (compare(part2, CMD_AUTO, 5, 3)) {
            button = AUTO;
        } else if (compare(part2, CMD_LOCK, 5, 3)) {
            button = LOCK;
        } else if (compare(part2, CMD_ONOFF, 5, 3)) {
            button = ONOFF;
        } else {
            button = unknown;
            print("error: part2 body");
        }

        return button;
    }

    String getButtonStr(eButton button) {
        switch (button) {
        case BRIGH:
            return "BRIGHT";
        case TIMER:
            return "TIMER";
        case SLEEP:
            return "SLEEP";
        case ANTI:
            return "ANTI";
        case TURBO:
            return "TURBO";
        case FAN:
            return "FAN";
        case AUTO:
            return "AUTO";
        case LOCK:
            return "LOCK";
        case ONOFF:
            return "ONOFF";
        case unknown:
            return "unknown";
        default:
            return "?????";
        }
    }

private:

    void print(String s) {
        if (debugEnabled) {
            debug->print(s);
        }
    }

    void print(int i) {
        if (debugEnabled) {
            debug->print(i);
        }
    }

    bool isEqual(unsigned long value, unsigned long target) {
        return (target - OFFSET <= value && value <= target + OFFSET);
    }

    bool compare(byte *A, const byte *B, byte startA, byte len) {
        unsigned Bi = 0;
        for (unsigned i = startA; i < startA + len; i++, Bi++) {
            if (A[i] != B[Bi])
                return false;
        }
        return true;
    }

}
;

#endif // __DAIKIN_RECEIVER_CLASS

