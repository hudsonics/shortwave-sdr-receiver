/*
 * display.h
 * =========
 *
 * Display module for SSD1309 128x64 OLED via u8g2 library
 * - Manages rendering user interface on display
 * - Page buffer mode used to minimise memory usage
 *
 * Display layout (128x64 pixels):
 * - 1px border around edge of display
 * - Rows 0-22:  Frequency, mode, filter width, cursor indicator
 * - Row 25:     Filter passband bracket
 * - Rows 32-62: FFT waterfall spectrum display
 *
 * Cursor position (set via rotary encoder):
 * - 0-7: Frequency step (10MHz to 1Hz steps)
 * - 8:   Mode (AM/USB/LSB/CW/FM)
 * - 9:   Filter width (NARROW/NORMAL/WIDE)
 */

#ifndef display_h
#define display_h
#include "u8g2_rx231.h"

#define MIN_CURSOR_POS  0
#define MAX_CURSOR_POS  9

#define CHAR_WIDTH  9

#define FREQ_X_POS  2
#define FREQ_Y_POS  10

#define MODE_X_POS  2
#define MODE_Y_POS  22

#define FILTER_WIDTH_X_POS  127
#define FILTER_WIDTH_Y_POS  22

#define FILTER_HEIGHT 4
#define FILTER_Y_POS  25
#define FILTER_X_POS  64

#define WATERFALL_RESOLUTION  200

class Display {
    public:
        void init();
        void set_cursor_pos(int8_t);
        uint8_t get_cursor_pos();
        void update(uint8_t *magnitudes);

    private:
        u8g2_t lcd;
        uint8_t cursor_pos;
        void draw_frequency();
        void draw_mode();
        void draw_filter_width();
        void draw_filter();
        void draw_cursor();
        void draw_waterfall(uint8_t *magnitudes);
};

#endif
