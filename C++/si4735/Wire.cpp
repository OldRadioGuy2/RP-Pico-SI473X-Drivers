/*
* Wire.cpp - Implementation of the TwoWire interface
*
*/
#include "Wire.h"
#include "pico/binary_info.h"


TwoWire::TwoWire() { 
    i2c = i2c_default; // AF Should be able to pass this in.
    i2c_init(i2c, 10000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
    // Make the I2C pins available to picotool
    
}

void TwoWire::begin(void){
    tx_buffer_len=0;
    tx_buffer_overrun = false;
    rx_buffer_len=0;
    rx_buffer_idx = 0;
    //rx_buffer_overrun = false;
    
}
void TwoWire::begin(uint8_t slave_addr){
    begin();
    i2c_set_slave_mode(i2c,true, slave_addr);

}
void TwoWire::end(void){
    // RP Pico does not support

}
size_t TwoWire::requestFrom(uint8_t addr, size_t quantity, bool stop){
    if (quantity == sizeof(rx_buffer) ) {
        printf("Error: RequestFrom quantity: %d exceeds rx_buffer size.\n", quantity); 
    }
    current_bus_address = addr;
    rx_buffer_len= rx_buffer_idx = 0;
    int result= i2c_read_timeout_us(i2c, addr, rx_buffer, quantity, stop, rx_timeout);
    if (result < 0){
         printf("Error: i2c_read_timeout_us returned ");
        switch (result) {
            case PICO_ERROR_TIMEOUT:
                printf("Timeout\n");
                break;
            case PICO_ERROR_GENERIC:
                printf("Generic Error\n");
                break;
            default:
                printf("Unexpected error: %d\n", result);
                break;
        
        }
        last_error = result;
        return 0;
    }
    printf("Received ");
    for (uint i=0; i < result; i++) {printf("%02x ", rx_buffer[i]);}
    printf("\n");
    return rx_buffer_len = result;
}

uint8_t TwoWire::read(void){
    if (rx_buffer_idx >= rx_buffer_len  ) {
        printf("Error: Wire Read attempt to read past buffer end: %d.\n", rx_buffer_len); 
    }    
    return rx_buffer[rx_buffer_idx++];
}

void TwoWire::beginTransmission(uint8_t addr){
    tx_buffer_len = 0;
    tx_buffer_overrun = false;
    current_bus_address=addr;
    return;
}

uint TwoWire::endTransmission(bool stop){
    printf("Sending ");
    for (uint i=0; i < tx_buffer_len; i++) {printf("%02x ", tx_buffer[i]);}
    printf("\n");
int result= i2c_write_blocking(i2c, current_bus_address, tx_buffer, tx_buffer_len,stop);
return result;
}

size_t TwoWire::write(uint8_t value){
    if (tx_buffer_len < sizeof(tx_buffer)) {
        tx_buffer[tx_buffer_len++] = value;
        
    }
    else {
        tx_buffer_overrun=true;
    }
    return 1;
}
size_t TwoWire::write(char * str){
    unsigned int count= 0;
    for (; *str != '\0' && !tx_buffer_overrun; count++) {
        write((uint8_t) *str++);
    }
    return count;
}
size_t TwoWire::write(uint8_t * data, size_t len){
    unsigned int count= 0;
    for (; count < len && !tx_buffer_overrun; count++) {
        write(*data++);
    }
    return count;
}
int TwoWire::available(void){
    return rx_buffer_len;
}


void TwoWire::setClock(uint32_t clock){
    i2c_set_baudrate(i2c, clock);
}
void TwoWire::onReceive(void (*)(int)){

}
void TwoWire::onReceive(void (*)(size_t)){

}
void TwoWire::onRequest(void (*)(void)){

}
void TwoWire::setWireTimeout(uint32_t timeout, bool reset_on_timeout){

}
void TwoWire::clearWireTimeoutFlag(void){

}
bool getWireTimeoutFlag(void){
    return false;
}
 