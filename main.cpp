#include <Arduino.h>
#include <avr/sleep.h>
#include <Wire.h>
#include "sh1106.h"
#include "bme280.h"
#include "rtc.h"
#include "dcf77.h"
#include "sgp40.h"


// Button for screen switch.
#define SWITCH_PIN	9

// Communication LED_PIN
#define LED_PIN     7



// External interrupt from RTC 1Hz signal.
ISR(INT1_vect)
{
	// Just wake up controller.
}




void setup()
{
    // Disable interrupts
    cli();

    // Enable external interrupt rising edge at INT1 pin
	EICRA |= 0x0C;
	// Activate extern interrupt INT1
	EIMSK = 0x02;
	// Reset both extern interrupt flags.
	EIFR  = 0x03;


    // Button for display view switch
    pinMode(SWITCH_PIN, INPUT);
    // ACtivate pull up.
    digitalWrite(SWITCH_PIN, true);

    // Communication LED.
    pinMode(LED_PIN, OUTPUT);


    // Enable interrupts
    sei();
}



void loop()
{
    // DCF77 to get the time from Frankfurt.
    DCF77 dcf77;

    // Get time from Frankfurt.
    digitalWrite(LED_PIN, true);
    bool time_received = dcf77.syncronize_time();
    digitalWrite(LED_PIN, false);


    // Arduino I2C framework. Do not start I2C before DCF77 has finished!
    Wire.begin();

    // RTC and sensors.
    BME280 bme280;
    SGP40Sensor sgp40;
    RTC rtc;


    // Set time in case of successful transfer.
    if(time_received)
    {
        rtc.hours = dcf77.hours;
        rtc.minutes = dcf77.minutes;
        rtc.seconds = 0;
        rtc.day_of_month = dcf77.day_of_month;

        rtc.week_day = dcf77.week_day;
        rtc.month = dcf77.month;
        rtc.year = dcf77.year;

        rtc.set_data();
    }


    // Controls SH1106 controller for OLED display.
    SH1106 sh1106(0x3C);

    // Active screen index.
    uint8_t active_screen = 1;
    // Time when to toggle the display.
    uint8_t display_toggle = 59;


    // Enter main process.
    while(true)
    {
        sh1106.clear_oled_buffer();

        // Screen print buffer.
        char string[20];


        // Receive temperature, humidity and VOC.
        // For precision the VOC value needs to be read every loop
        int16_t temperature;
        uint16_t pressure, humidity;
        uint16_t voc_index = 0;

        bme280.get_measurements(&temperature, &pressure, &humidity);
        sgp40.get_voc_index((float)temperature / 100.0, (float)humidity / 1000.0, &voc_index);


        // Get current RTC data.
        rtc.update();

        // Show data depending on screen configuration.
        switch(active_screen)
        {
            case 0:
                //Print humidity.
                sprintf(string, "%i.%i %% R.H.", humidity / 1000, (humidity % 1000) / 100);
                sh1106.draw_string(0, 60, string);

                // Print temperature.
                sprintf(string, "%i.%i $C", temperature / 100, (temperature % 100) / 10);
                sh1106.draw_string(0, 40, string);

                // Print VOC index.
                sprintf(string, "%s%i", "VOC: ", voc_index);
                sh1106.draw_string(0, 20, string);
                break;

            case 1:
                // Print date.
                sprintf(string, "%i%i.%i%i.%i%i", rtc.day_of_month / 10, rtc.day_of_month % 10,
                                            rtc.month / 10, rtc.month % 10,
                                            rtc.year / 10, rtc.month % 10);
                sh1106.draw_string(0, 55, string);

                // Print time.
                sprintf(string, "%i%i:%i%i:%i%i  %s", rtc.hours / 10, rtc.hours % 10,
                                                    rtc.minutes / 10, rtc.minutes % 10,
                                                    rtc.seconds / 10, rtc.seconds % 10,
                                                    rtc.day_names[rtc.week_day - 1]);
                sh1106.draw_string(0, 25, string);
                break;
        }


        sh1106.flush_oled_buffer();

        // Screen toggles every minute, respective to last button input.
        if(rtc.seconds == display_toggle)
        {
            active_screen = !active_screen;
        }

        // Button input toggles screen manually.
        if(!digitalRead(SWITCH_PIN))
        {
            display_toggle = rtc.seconds;
            active_screen = ! active_screen;
        }

		// Enter sleep mode, RTC 1Hz trigger wakes up controller.
		SMCR |= (1 << 0);
		SMCR |= (1 << 2);
		sleep_cpu();
    }
}
