#ifndef RTC_H_
#define RTC_H_

#include <Wire.h>

#define RTC_I2C_ADDRESS 0x68



class RTC
{
    public:
        static const char* day_names[7];

        uint16_t year;
        uint16_t month;
        uint16_t day_of_month;
        uint16_t week_day;
        uint16_t hours;
        uint16_t minutes;
        uint16_t seconds;


        RTC();
        void set_data();
        void update();
        uint8_t bcd_to_dec(uint8_t bcd_value);
};


#endif
