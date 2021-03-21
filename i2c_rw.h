#include <unistd.h>

int8_t i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint16_t length);
int8_t i2c_write(uint8_t dev_addr, uint8_t reg_addr, const uint8_t* data, uint16_t length);
