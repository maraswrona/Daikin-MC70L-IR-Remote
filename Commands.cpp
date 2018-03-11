#ifndef __DAIKIN_COMMANDS_CLASS
#define __DAIKIN_COMMANDS_CLASS

#include <Arduino.h>

/*
 * Each message is a 128bit (16byte) sequence:
 * - PREFIX (3byte) + HEADER_BEGIN (2byte) + HEADER_BODY (3byte) = 8byte = 64bit
 * - PREFIX (3byte) +    CMD_BEGIN (2byte) +    CMD_BODY (3byte) = 8byte = 64bit
 */

const byte PREFIX[3] = { 0b10001000, 0b01011011, 0b11101001 };

const byte HEADER_BEGIN[2] = { 0b00001111, 0b00000000 };
const byte HEADER_BODY[3] = { 0b00000000, 0b00000000, 0b01001110 };

const byte CMD_BEGIN[2] = { 0b00000000, 0b11110001 };

const byte CMD_ONOFF[3] = { 0b10000000, 0b00000000, 0b01001000 };
const byte CMD_FAN[3] = { 0b00001000, 0b00000000, 0b10000100 };
const byte CMD_ANTI[3] = { 0b10010000, 0b00000000, 0b01011000 };
const byte CMD_SLEEP[3] = { 0b11101000, 0b00000000, 0b00010100 };
const byte CMD_BRIGH[3] = { 0b00011100, 0b00000000, 0b10010010 };
const byte CMD_TIMER[3] = { 0b11000000, 0b00000000, 0b00101000 };
const byte CMD_TURBO[3] = { 0b00010000, 0b00000000, 0b10011000 };
const byte CMD_AUTO[3] = { 0b01000000, 0b00000000, 0b11001000 };
const byte CMD_LOCK[3] = { 0b00101000, 0b00000000, 0b10100100 };

#endif // __DAIKIN_COMMANDS_CLASS

