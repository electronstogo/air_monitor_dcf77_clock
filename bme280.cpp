#include "bme280.h"
#include <util/delay.h>




BME280::BME280()
{
    // Do a sensor reset, through sending 0xB6 to register address 0xE0.
    uint8_t command_data[2] = {0xE0, 0xB6};
    Wire.beginTransmission(BME280_I2C_ADDRESS);
    Wire.write(command_data, 2);


    // Wait until adjust parameters are written to memory.
    while(true)
    {
        Wire.write(0xF3);
        Wire.endTransmission();
		Wire.requestFrom(BME280_I2C_ADDRESS, 1);

        if(!(Wire.read() & 0x01))
        {
            break;
        }

        _delay_ms(10);
    }

    Wire.endTransmission();


    // Get adjustment parameters from memory.
    this->get_adjustment();


    // Set sensor asleep, before configuring.
    Wire.beginTransmission(BME280_I2C_ADDRESS);
    Wire.write(0xF4);
    Wire.write(0b0);

    // trigger forced mode of bmp sensor.
    // configure all measurements, with oversampling rate 1.

    // Write humidity configuration.
    Wire.write(0xF2);
    Wire.write(0b1);

    // Write temperature / pressure configuration and meas mode.
    Wire.write(0xF4);
    Wire.write(0b00100101);
    Wire.endTransmission();

    // Disable filters.
    Wire.beginTransmission(BME280_I2C_ADDRESS);
    Wire.write(0xF5);
    Wire.write(0x0);
    Wire.endTransmission();
}




// Read adjustment parameters from sensor memory.
void BME280::get_adjustment()
{
	// Get first part of adjustment registers.
	uint8_t reg_data[26];

	Wire.beginTransmission(BME280_I2C_ADDRESS);
	Wire.write(0x88);
	Wire.endTransmission();


	Wire.requestFrom(BME280_I2C_ADDRESS, 26);

	for(uint16_t i = 0; i < 26; i++)
    {
        reg_data[i] = Wire.read();
    }

	this->adj_t1 = CONCAT_BYTES(reg_data[1], reg_data[0]);
	this->adj_t2 = (int16_t)CONCAT_BYTES(reg_data[3], reg_data[2]);
	this->adj_t3 = (int16_t)CONCAT_BYTES(reg_data[5], reg_data[4]);

	this->adj_p1 = CONCAT_BYTES(reg_data[7], reg_data[6]);
	this->adj_p2 = (int16_t)CONCAT_BYTES(reg_data[9], reg_data[8]);
	this->adj_p3 = (int16_t)CONCAT_BYTES(reg_data[11], reg_data[10]);
	this->adj_p4 = (int16_t)CONCAT_BYTES(reg_data[13], reg_data[12]);
	this->adj_p5 = (int16_t)CONCAT_BYTES(reg_data[15], reg_data[14]);
	this->adj_p6 = (int16_t)CONCAT_BYTES(reg_data[17], reg_data[16]);
	this->adj_p7 = (int16_t)CONCAT_BYTES(reg_data[19], reg_data[18]);
	this->adj_p8 = (int16_t)CONCAT_BYTES(reg_data[21], reg_data[20]);
	this->adj_p9 = (int16_t)CONCAT_BYTES(reg_data[23], reg_data[22]);

	this->adj_h1 = reg_data[25];


	// Get second part adjustment registers.
	Wire.beginTransmission(BME280_I2C_ADDRESS);
	Wire.write(0xE1);
	Wire.endTransmission();


	Wire.requestFrom(BME280_I2C_ADDRESS, 7);
    for(uint16_t i = 0; i < 7; i++)
    {
        reg_data[i] = Wire.read();
    }

	this->adj_h2 = (int16_t)CONCAT_BYTES(reg_data[1], reg_data[0]);
	this->adj_h3 = reg_data[2];

	int16_t calculation_buffer = (int16_t)(((int8_t)reg_data[3]) << 4);
	this->adj_h4 = calculation_buffer + (int16_t)(reg_data[4] & 0x0F);

	calculation_buffer = (int16_t)(((int8_t)reg_data[5]) << 4);
	this->adj_h5 = calculation_buffer + (int16_t)(reg_data[4] >> 4);

	this->adj_h6 = (int8_t)reg_data[6];
}




// Reads all sensor measurements.
void BME280::get_measurements(int16_t* temperature, uint16_t* pressure, uint16_t* humidity)
{
	// Read the humidity/temperature/pressure registers up from address 0xF7.
	// trigger a forced mode measurement.

	Wire.beginTransmission(BME280_I2C_ADDRESS);
	Wire.write(0xF4);
	Wire.write(0b00100101);

	// Wait until the measurement finish flag is set.
	while(true)
	{
        Wire.write(0xF3);
		Wire.endTransmission();
		Wire.requestFrom(BME280_I2C_ADDRESS, 1);

        if(Wire.read() & 0x08)
        {
            break;
        }

        _delay_ms(10);
	}

	Wire.endTransmission();

	uint8_t reg_data[8];
	uint32_t adc_value;

	Wire.beginTransmission(BME280_I2C_ADDRESS);
	Wire.write(0xF7);
	Wire.endTransmission();

	// Do a burst read for all measurements.
	Wire.requestFrom(BME280_I2C_ADDRESS, 8);
	for(uint16_t i = 0; i < 8; i++)
    {
        reg_data[i] = Wire.read();
    }


	// Calculate the temperature adc value.
	adc_value = ((uint32_t)reg_data[3]) << 12;
    adc_value += ((uint32_t)reg_data[4]) << 4;
    adc_value += (uint32_t)reg_data[5] >> 4;

	// Calculate the temperature measurement from adc value.
	*temperature = this->calculate_temperature((int32_t)adc_value);


	// Calculate the pressure adc value.
    adc_value = ((uint32_t)reg_data[0]) << 12;
    adc_value += ((uint32_t)reg_data[1]) << 4;
    adc_value += (uint32_t)reg_data[2] >> 4;

	// Calculate pressure measurement from adc value.
	*pressure = this->calculate_pressure(adc_value);


	// Calculate the humidity adc value.
    adc_value = ((uint32_t)reg_data[6]) << 8;
    adc_value += (uint32_t)reg_data[6];

	*humidity = this->calculate_humidity((int32_t)adc_value);
}


// Calculates the temperature from measurement.
int32_t BME280::calculate_temperature(int32_t adc_temp)
{
	int32_t t1 = (int32_t)this->adj_t1;
	int32_t t2 = (int32_t)this->adj_t2;
	int32_t t3 = (int32_t)this->adj_t3;


  	int32_t var1 = ((((adc_temp >> 3) - (t1 << 1))) * t2) >> 11;
  	int32_t var2 = (((((adc_temp >> 4) - t1) * ((adc_temp >> 4) - t1)) >> 12) * t3) >> 14;

    this->t_fine = var1 + var2;

    int32_t temperature =  (this->t_fine * 5 + 128) >> 8;

    // Check min / max limit and return.
    return temperature < -4000 ? -4000 : temperature > 8500 ? 8500 : temperature;
}




// Calculates the pressure from measurement.
uint32_t BME280::calculate_pressure(uint32_t adc_pressure)
{
	int32_t var1, var2, var3, var4;
    uint32_t var5, pressure;

    var1 = (((int32_t)this->t_fine) / 2) - (int32_t)64000;
    var2 = (((var1 / 4) * (var1 / 4)) / 2048) * ((int32_t)this->adj_p6);
    var2 = var2 + ((var1 * ((int32_t)this->adj_p5)) * 2);
    var2 = (var2 / 4) + (((int32_t)this->adj_p4) * 65536);
    var3 = (this->adj_p3 * (((var1 / 4) * (var1 / 4)) / 8192)) / 8;
    var4 = (((int32_t)this->adj_p2) * var1) / 2;
    var1 = (var3 + var4) / 262144;
    var1 = (((32768 + var1)) * ((int32_t)this->adj_p1)) / 32768;

	// Prevent zero division.
	if(var1 == 0)
    {
         return 300;
    }

    var5 = (uint32_t)((uint32_t)1048576) - adc_pressure;
    pressure = ((uint32_t)(var5 - (uint32_t)(var2 / 4096))) * 3125;

    if (pressure < 0x80000000)
    {
        pressure = (pressure << 1) / ((uint32_t)var1);
    }
    else
    {
        pressure = (pressure / (uint32_t)var1) * 2;
    }

    var1 = (((int32_t)this->adj_p9) * ((int32_t)(((pressure / 8) * (pressure / 8)) / 8192))) / 4096;
    var2 = (((int32_t)(pressure / 4)) * ((int32_t)this->adj_p8)) / 8192;
    pressure = (uint32_t)((int32_t)pressure + ((var1 + var2 + this->adj_p7) / 16));

	if(pressure < 30000)
	{
		return 300;
	}
	else if(pressure > 110000)
	{
		return 1100;
	}

	return pressure / 100;
}



// Calculates the humidity from measurement.
uint32_t BME280::calculate_humidity(int32_t adc_humidity)
{
	int32_t h1 = (int32_t)this->adj_h1;
	int32_t h2 = (int32_t)this->adj_h2;
	int32_t h3 = (int32_t)this->adj_h3;
	int32_t h4 = (int32_t)this->adj_h4;
	int32_t h5 = (int32_t)this->adj_h5;
	int32_t h6 = (int32_t)this->adj_h6;


	int32_t v_x1_u32r = (this->t_fine - ((int32_t)76800));

	v_x1_u32r = (((((adc_humidity << 14) - (h4 << 20) - (h5 * v_x1_u32r) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * h6) >> 10) * (((v_x1_u32r * h3) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * h2 + 8192) >> 14)));

	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * h1) >> 4));

	v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
	v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;

	uint32_t humidity = (uint32_t)(v_x1_u32r >> 12);

	return humidity;
}
