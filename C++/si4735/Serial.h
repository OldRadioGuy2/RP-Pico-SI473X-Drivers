/*
* Implementation of the Arduino Serial interface
* for the Raspberry Pi Pico
*
*   Copyright (c) 2023 Alan Finger*
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "
* "Software"), to deal in the Software without restriction
* including without limitation the rights to use, copy, modify,
* distribute, sublicense, and/or sell copies of
* the Software, and to permit persons to whom the
* Software is furnished
* to do so, subject to the following conditions:
* 
*
*
*/

#include <stdlib.h>
#include <cstdio>
#include "pico/stdlib.h"

#define DEFAULT_UART uart0
#define DEFAULT_UART_TX_PIN 0
#define DEFAULT_UART_RX_PIN 1

class PicoSerial
{
    public:
    static PicoSerial& get_instance(uart_inst_t * uart_p=DEFAULT_UART, int tx_pin=DEFAULT_UART_TX_PIN, int rx_pin=DEFAULT_UART_RX_PIN )
    {
        // If the instance doesn't exist, create it
        if (!instance) {
            instance = new PicoSerial();
        }
        instance->uart_inst_p = uart_p;
        
        return *instance;
    }

    private:
        PicoSerial() {}
        ~PicoSerial() {}
        static PicoSerial* instance;
        uart_inst_t * uart_inst_p;
        int tx_pin;
        int rx_pin;
        int timeout= 1000;
    public:
    int available() {return uart_is_enabled(instance->uart_inst_p);}
    int availableForWrite() {return uart_is_writable(instance->uart_inst_p);}
    void setTimeout(int timeout) {instance->timeout = timeout;} //milliseconds
    void begin( unsigned int speed, unsigned int config=0, int tx_pin=DEFAULT_UART_TX_PIN, int rx_pin=DEFAULT_UART_RX_PIN ) {stdio_uart_init_full(instance->uart_inst_p, speed, tx_pin, rx_pin);}
    void end() {uart_deinit(instance->uart_inst_p);}
    void flush() {stdio_flush();}
    size_t print(const int i) {return printf("%d ", i);}
    size_t print (const double d) {return printf("%6.2f", d);}
    size_t print (const char c) {return printf("%c", c);}
    size_t print (const char *s) {return printf("%s", s);}
    size_t println(const int i) {return printf("%d\n", i);}
    size_t println (const double d) {return printf("%6.2f\n", d);}
    size_t println (const char c) {return printf("%c\n", c);}
    size_t println (const char *s) {return printf("%s\n", s);}
    int read() {return (uart_is_readable_within_us(instance->uart_inst_p, PicoSerial::timeout*1000)) ? uart_getc(instance->uart_inst_p) : -1;}
    int readBytes(char *buf, int len,  int terminator =-1) ;
    int readBytesUntil(char c, char *buf, int len);
    int readString(char *buf, int len);
    int readStringUntil(char c, char *buf, int len);
    size_t write(char c);
    size_t write(char *s);
    size_t write(char *buf, int len);
    //bool if(Serial)
    //peek();
    //serialEvent();
};

