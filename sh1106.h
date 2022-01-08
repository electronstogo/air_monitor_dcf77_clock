#ifndef SH1106_H_
#define SH1106_H_



#define SH1106_I2C_ADDRESS 	0x3C//0x3D


#define OLED_WIDTH 132
#define OLED_HEIGHT 64



class SH1106
{
    private:
        void write_command(unsigned char command);
        void write_data(uint8_t *data, uint16_t length);
        void set_page(uint16_t page);
        void draw_point(uint16_t x, uint16_t y);
        void draw_letter(uint16_t x, uint16_t y, uint16_t index);

        unsigned char i2c_address;

    public:
        SH1106(uint8_t sh1106_i2c_address = SH1106_I2C_ADDRESS);
        void clear_oled_buffer();
        void flush_oled_buffer();
        void draw_string(uint16_t x_pos, uint16_t y_pos, const char *string);
};




#endif
