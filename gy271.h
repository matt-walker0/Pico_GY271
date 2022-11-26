#ifndef gy271_included
#define gy271_included

#include "hardware/i2c.h"

bool MAG_Init(i2c_inst_t* _i2c_port_, uint8_t _device_address_);
bool MAG_Setup();
bool MAG_Heading(float *heading);

#endif