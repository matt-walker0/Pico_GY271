#include <stdio.h>
#include "math.h"
#include "gy271.h"

#define RAD_TO_DEG 57.29578f


bool gy271_read(int16_t axis[3]);


// Global variables (static prevents contamination)
static uint8_t device_address; // GY271 typically on 0x0D
static i2c_inst_t* i2c_port; // i2c


bool MAG_Init(i2c_inst_t* _i2c_port_, uint8_t _device_address_) 
{
    i2c_port = _i2c_port_;
    device_address = _device_address_;

    // Send startup to device. 
    return(MAG_Setup());
}


// Flash and confirm GY271 config. Returns TRUE if successful.
bool MAG_Setup() {
    // https://www.filipeflop.com/img/files/download/Datasheet-QMC5883L-1.0%20.pdf
    // 0b00xxxxxx  -> 512 over sample rate.
    // 0bxx00xxxx  -> +-2 gauss range.
    // 0bxxxx01xx  -> 10Hz output rate.
    // 0bxxxxxx01  -> Continuous mode.
    const uint8_t cfg[] = {0x09, 0b00000101};
    i2c_write_timeout_us(i2c_port, device_address, cfg, 2, false, 5000);


    // Confirm registers are correctly set. (registers set above are r/w, can use same addr)
    // To read from GY-271 must write register of interest and invalid byte.
    uint8_t temp[] = {0};
    i2c_write_timeout_us(i2c_port, device_address, cfg, 1, true, 1000);
    i2c_read_timeout_us(i2c_port, device_address, temp, 1, false, 1000);
    if(temp[0] != cfg[1]) { return(false); }

    return(true);
}



bool MAG_Heading(float *heading)
{
    int16_t mag_axis[3]; 

    if(gy271_read(mag_axis) == false) {
        return(false); // failed to read.
    }

  
    printf("MAG 1:%d  MAG 2:%d  MAG 3:%d\n", mag_axis[0], mag_axis[1], mag_axis[2]);


    *heading = atan2f( mag_axis[1], mag_axis[0]) * RAD_TO_DEG;

    // if(*heading < 0) {
    //     *heading += 360;
    // }
    return(true);
}


// Returns TRUE if successful read.
// Array is X,Y,Z in that order.
bool gy271_read(int16_t axis[3])
{
    const uint8_t data_addr = 0x00; // Beginning of data.
    uint8_t buff[6];
    if(i2c_write_timeout_us(i2c_port, device_address, &data_addr, 1, true, 1000) != 1) { return(false); };
    if(i2c_read_timeout_us(i2c_port, device_address, buff, 6, false, 5000) != 6) { return(false); }

    for(uint8_t i = 0; i < 3; i++) {
        // Values are into LSB then MSB for each of X,Y,Z.
        // Hence read two bytes per int16 and bitshift latter byte. 
        axis[i] = (buff[i*2] | buff[i*2+1] << 8);
    }
    return(true);
}