# air_monitor_dcf77_clock


Combination of air monitor that shows a VOC index, humidity and temperature, and a clock that uses a DCF77 module to receive the current time from Frankfurt, when the device is started. 
A LED is used to show the DCF77 communication at start (display is turned off until the time was received, the signal will be disturbed by I2C bus).
A switch is used to toggle the display view.


This circuit uses a 1.3 inch OLED display to show the following values:
-----------------------------------------------------------------------

- VOC index from Sensirion
- Relative humidity [%]
- Temperature [°C]
- Time [hh:mm:ss]
- Week day
- Date [dd.mm.yyyy]


Components in use:
------------------

- Sensirion SGP40 sensor
- Bosch BME280 sensor
- DS3231 RTC modul
- 1.3 inch OLED modul
- Arduino Nano
- DCF77 module
