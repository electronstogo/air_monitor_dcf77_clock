#ifndef BME280_H_
#define BME280_H_


#include <Wire.h>

#define BME280_I2C_ADDRESS 	0x76

#define CONCAT_BYTES(msb, lsb)	(((uint16_t)msb << 8) | (uint16_t)lsb)


class BME280
{
	private:
		// Adjustment parameters from sensor memory.
		uint16_t adj_t1;
		int16_t adj_t2;
		int16_t adj_t3;

		uint16_t adj_p1;
		int16_t adj_p2;
		int16_t adj_p3;
		int16_t adj_p4;
		int16_t adj_p5;
		int16_t adj_p6;
		int16_t adj_p7;
		int16_t adj_p8;
		int16_t adj_p9;

		uint8_t adj_h1;
		int16_t adj_h2;
		uint8_t adj_h3;
		int16_t adj_h4;
		int16_t adj_h5;
		int8_t adj_h6;

		int32_t t_fine;

		void get_adjustment();
		int32_t calculate_temperature(int32_t adc_temperature);
		uint32_t calculate_pressure(uint32_t adc_pressure);
		uint32_t calculate_humidity(int32_t adc_humidity);

	public:
		BME280();
		void init();
		void get_measurements(int16_t* temperature, uint16_t* pressure, uint16_t* humidity);

};




#endif
