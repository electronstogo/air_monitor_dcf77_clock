#include "rtc.h"



const String RTC::day_names[7] = {"Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"};
const String RTC::month_names[12] = {"Jan", "Feb", "Mär", "Apr", "Mai", "Jun", "Jul", "Aug", "Sept", "Okt", "Nov", "Dez"};

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
    Wire.write(dec_to_bcd(seconds));
    Wire.write(dec_to_bcd(minutes));

    // set hours, and activate 24 hr mode.
    Wire.write(dec_to_bcd(hours));

    Wire.write(week_day & 0x7);
    Wire.write(dec_to_bcd(day_of_month));
    Wire.write(dec_to_bcd(month));
    Wire.write(dec_to_bcd(year));

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
    seconds = bcd_to_dec(Wire.read() & 0x7F);
    minutes = bcd_to_dec(Wire.read() & 0x7F);
    hours = bcd_to_dec(Wire.read() & 0x1F);
    week_day = bcd_to_dec(Wire.read() & 0x7);
    day_of_month = bcd_to_dec(Wire.read() & 0x3F);
    month = bcd_to_dec(Wire.read() & 0x1F);
    year = bcd_to_dec(Wire.read());
}


String RTC::get_week_day()
{
    return day_names[week_day - 1];
}

// Calculate BCD format to DEC format.
uint8_t RTC::bcd_to_dec(uint8_t value)
{
    return ((value & 0x0F) + (value >> 4) * 10);
}

// Calculate DEC format to BCD format.
uint8_t RTC::dec_to_bcd(uint8_t value)
{
    return ((value / 10 * 16) + (value % 10));
}
