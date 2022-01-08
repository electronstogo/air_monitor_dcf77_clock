#include "rtc.h"



const char* RTC::day_names[7] = {"MO", "DI", "MI", "DO", "FR", "SA", "SO"};

RTC::RTC()
{
    // RTC configuration.
    Wire.beginTransmission(RTC_I2C_ADDRESS);
    Wire.write(0x0E);
    Wire.write(0x40);
    Wire.endTransmission();
}


// Write date / time data to RTC registers.
void RTC::set_data()
{
    Wire.beginTransmission(RTC_I2C_ADDRESS);
    Wire.write(0x0);
    Wire.write(((this->seconds / 10) << 4) + (this->seconds % 10));
    Wire.write(((this->minutes / 10) << 4) + (this->minutes % 10));

    // set hours, and activate 24 hr mode.
    uint8_t hour_byte = ((this->hours / 10) << 4) + (this->hours % 10);
    Wire.write(hour_byte & 0b10111111);

    Wire.write(this->week_day);
    Wire.write(((this->day_of_month / 10) << 4) + (this->day_of_month % 10));
    Wire.write(((this->month / 10) << 4) + (this->month % 10));
    Wire.write(((this->year / 10) << 4) + (this->year % 10) + 0x80);

    Wire.endTransmission();
}


// Read time data from RTC controller DS3231.
void RTC::update()
{
    // Start reading the RTC registers from address 0x0.
    Wire.beginTransmission(RTC_I2C_ADDRESS);
    Wire.write(0x0);
    Wire.endTransmission();


    // Get 3 bytes (seconds register, minutes register, hours register)
    Wire.requestFrom(RTC_I2C_ADDRESS, 7);
    this->seconds = bcd_to_dec(Wire.read() & 0x7F);
    this->minutes = bcd_to_dec(Wire.read() & 0x7F);
    this->hours = bcd_to_dec(Wire.read() & 0x7F);
    this->week_day = bcd_to_dec(Wire.read() & 0x7);
    this->day_of_month = bcd_to_dec(Wire.read() & 0x3F);
    this->month = bcd_to_dec(Wire.read() & 0x3);
    this->year = bcd_to_dec(Wire.read() & 0x7F);
}


// Calculate BCD format to DEC format.
uint8_t RTC::bcd_to_dec(uint8_t bcd_value)
{
    return (bcd_value & 0x0F) + (bcd_value >> 4) * 10;
}
