/*
* Wire.h defines the Arduino I2C interface
* Copyright (c) 2023 Alan Finger
*
*/
#ifndef WIRE_H
#define WIRE_H 
#include <stdlib.h>
#include "ArduinoShim.h"
#include "pico.h"
#include "pico/time.h"
#include <pico/stdlib.h>
#include <hardware/structs/i2c.h>
#include <hardware/i2c.h>
#include <iostream>
#include <queue>

#define TX_BUFFER_SIZE 256
#define RX_BUFFER_SIZE 256


class TwoWire  {
    private:
    i2c_inst_t * i2c;
    queue<uint8_t> rx_queue;
    queue<uint8_t> tx_queue;
    uint8_t tx_buffer[TX_BUFFER_SIZE];
    uint tx_buffer_len=0;
    bool tx_buffer_overrun = false;
    uint8_t rx_buffer[RX_BUFFER_SIZE];
    uint rx_buffer_idx=0;
    uint rx_buffer_len=0;
    uint rx_buffer_end = 0;
    uint8_t current_bus_address;
    int last_error = 0;
    int rx_timeout = 10000;


    public: 
    TwoWire();
    void begin(void);
    void begin(uint8_t slave_addr);
    void end(void);
    size_t requestFrom(uint8_t addr, size_t quantity, bool stop=true);
    void beginTransmission(uint8_t addr);
    uint endTransmission(bool stop=true);
    size_t write(uint8_t value);
    size_t write(char * str);
    size_t write(uint8_t * data, size_t len);
    int available(void);
    uint8_t read(void);
    void setClock(uint32_t clock);
    void onReceive(void (*)(int));
    void onReceive(void (*)(size_t));
    void onRequest(void (*)(void));
    void setWireTimeout(uint32_t timeout=10000, bool reset_on_timeout=true);
    void clearWireTimeoutFlag(void);
    bool getWireTimeoutFlag(void);
    };
#endif