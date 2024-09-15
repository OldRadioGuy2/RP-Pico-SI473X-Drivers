/*
* Arduino Shim - Holds definitions and functions that were defined 
* in the Arduino library
*/

/*
* Typedefs that were originally in Arduino.h
*/
#pragma once
#include <stdlib.h>
#include <string>
#include <cstddef>
#include "pico.h"
#include "pico/time.h"
#include <pico/stdlib.h>
#include <hardware/structs/i2c.h>
//#include "hardware/i2c.h"
using namespace std;

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
//typedef uint8_t byte;
typedef unsigned int uint;
//typedef signed int int32_t;
//typedef unsigned int uint32_t;
//#define NULL 0 

#define OUTPUT GPIO_OUT
#define INPUT GPIO_IN
#define HIGH true
#define LOW false
#define B10000000 0b10000000

inline void pinMode(uint pin, uint mode, bool pullup=true) {gpio_init(pin); gpio_set_dir(pin, mode);if (pullup) {gpio_pull_up(pin);} }
inline void digitalWrite(uint pin, bool value) {gpio_put(pin, value);}
inline void delayMicroseconds(uint64_t delay) {sleep_us( delay);}
inline void delay(uint64_t delay) {sleep_ms( delay);}

inline int digitalPinToInterrupt(uint pin) {return 0;}  /*for now*/

inline uint32_t millis() {return time_us_64()/1000;}

inline int digitalPinToInterrupt(int pin) {return pin;}
#define IRQ_LOW GPIO_IRQ_LEVEL_LOW
#define IRQ_HIGH GPIO_IRQ_LEVEL_HIGH
#define IRQ_RISING GPIO_IRQ_EDGE_RISE
#define IRQ_FALLING GPIO_IRQ_EDGE_FALL
inline void attachInterrupt(int interruptPin, 	gpio_irq_callback_t interrupt_handler, uint32_t event_mask) {
    gpio_set_irq_enabled_with_callback(interruptPin, event_mask, true, interrupt_handler);
}
