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

#include "Serial.h"

int PicoSerial::readBytes(char *buf, int len, int terminator) {
    int i;
    for (i = 0; i < len && uart_is_readable_within_us(instance->uart_inst_p, PicoSerial::timeout*1000); i++) {
        *buf = uart_getc(instance->uart_inst_p);
        if ( *buf++ == terminator ) {
            break;
        }
    }
    return i;
}

int PicoSerial::readBytesUntil(char c, char *buf, int len) {
    return readBytes(buf, len, c);
    
}
int PicoSerial::readString(char *buf, int len) {
    return readBytes(buf, len);
}


int PicoSerial::readStringUntil(char c, char *buf, int len) {
    return readBytes(buf, len, c);
    
}
size_t PicoSerial::write(char c) {
    uart_putc(instance->uart_inst_p, c );
    return 1;
}

size_t PicoSerial::write(char *s) {
    size_t len = 0;
    while (*s) {
        char c=*(s++);
        uart_putc(instance->uart_inst_p, c );
        len += (c == '\n') ? 2: 1;
        }
    return len;
}

size_t PicoSerial::write(char *buf, int len) {
    for (int i=0; i < len; i++) {
        uart_putc(instance->uart_inst_p, *(buf++) );
        }
    return len;
}

