#include "sgp40.h"
#include <math.h>
#include <stdlib.h>
#include <util/delay.h>




SGP40Sensor::SGP40Sensor()
{
    // Get the VOC index algorithm parameters.
    VocAlgorithm_init(&this->voc_algorithm_parameters);
}



// Returns the raw measurement from SGP40 sensor, after sending the current temperature and humidity.
bool SGP40Sensor::do_raw_measurement(float temperature, float humidity, uint16_t* raw_value)
{
    uint8_t i2c_command[8];

    i2c_command[0] = 0x26;
    i2c_command[1] = 0x0F;


    // Send the themperature & humidity to the sensor.

    // Convert data into ticks (see datasheet 1.0 page 13).
    uint16_t tick_value = (uint16_t)((humidity * 65535) / 100 + 0.5);
    i2c_command[2] = tick_value >> 8;
    i2c_command[3] = tick_value && 0xFF;
    i2c_command[4] = this->generate_crc(i2c_command + 2, 2);

    tick_value = (uint16_t)(((temperature + 45) * 65535) / 175);
    i2c_command[5] = tick_value >> 8;
    i2c_command[6] = tick_value && 0xFF;
    i2c_command[7] = this->generate_crc(i2c_command + 5, 2);

    Wire.beginTransmission(SGP40_I2C_ADDRESS);
    Wire.write(i2c_command, 8);
    Wire.endTransmission();

    // Wait until measurement is finished.
    _delay_ms(25);

    // Get the raw data from sensor.
    Wire.requestFrom(SGP40_I2C_ADDRESS, 3);
    uint8_t i2c_receive[2];

    i2c_receive[0] = Wire.read();
    i2c_receive[1] = Wire.read();
    uint8_t crc = Wire.read();


    // Calculate the VOC raw value
    *raw_value = i2c_receive[0] << 8;
    *raw_value += i2c_receive[1];


    // CRC check of received raw data.
    if(this->generate_crc(i2c_receive, 2) != crc)
    {
        return false;
    }

    return true;
}



// Read the VOC index from SGP40.
bool SGP40Sensor::get_voc_index(float temperature, float humidity, uint16_t* voc_index)
{
    int32_t voc_index_buffer;
    uint16_t raw_value;

    if(!do_raw_measurement(temperature, humidity, &raw_value))
    {
        return false;
    }

    // call the Sensirion SGP40 algorithm function to get the VOC index.
    VocAlgorithm_process(&voc_algorithm_parameters, raw_value, &voc_index_buffer);
    *voc_index = (uint16_t)voc_index_buffer;

    return true;
}




// Calculates the CRC checksum.
uint8_t SGP40Sensor::generate_crc(uint8_t *data, uint16_t data_length)
{
    uint8_t crc = 0xFF;

    for(uint16_t i = 0; i < data_length; i++)
    {
        //crc ^= (data >> (i * 8)) & 0xFF;
        crc ^= data[i];

        for(uint16_t b = 0; b < 8; b++)
        {
            if(crc & 0x80)
            {
                crc = (crc << 1) ^ SGP40_CRC8_POLYNOMIAL;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}
