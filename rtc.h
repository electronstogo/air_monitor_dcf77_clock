#ifndef RTC_H_
#define RTC_H_

#include <Wire.h>

#define RTC_I2C_ADDRESS 0x68



class RTC
{
    public:
        static const String day_names[7];
        static const String month_names[12];

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
        String get_week_day();

    private:
        uint8_t bcd_to_dec(uint8_t value);
        uint8_t dec_to_bcd(uint8_t value);
};


#endif
