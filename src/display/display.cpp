/*
 * display.cpp
 * ===========
 *
 * Display module for SSD1309 128x64 OLED via u8g2 library
 * - Initialises display with page buffer mode and custom font
 * - Renders frequency, mode, filter width, cursor, filter bracket and waterfall
 * - Contrast set low (1) to reduce charge pump switching noise on 3V3 rail
 * - Screen refresh rate adjusted via 0xD5/0xF1 command to reduce charge pump frequency
 */
 
#include "display.h"
#include "dsp.h"
#include "globals.h"
#include "u8g2_rx231.h"
#include "vfo.h"
#include "fft.h"

extern "C" {
    #include "r_smc_entry.h"
}

// Custom font - contains only characters used to save memory
// 0123456789.AMFMUSBLCWNORWIDE
const uint8_t sdr_font[834] U8G2_FONT_SECTION("sdr_font") = "\200\0\3\5\4\4\3\5\5\10\10\0\0\10\10\10\10\1w\2\210\3%\0\5\0\304\31\1\5\0\304"
                                                            "\31\2\5\0\304\31\3\5\0\304\31\4\5\0\304\31\5\5\0\304\31\6\5\0\304\30\7\5\0\304\31\10"
                                                            "\5\0\304\31\11\5\0\304\31\12\5\0\304\31\13\5\0\304\31\14\5\0\304\31\15\5\0\304\31\16\5\0"
                                                            "\304\31\17\5\0\304\31\20\5\0\304\31\21\5\0\304\31\22\5\0\304\31\23\5\0\304\31\24\5\0\304\31"
                                                            "\25\5\0\304\31\26\5\0\304\31\27\5\0\304\31\30\5\0\304\31\31\5\0\304\31\32\5\0\304\31\33\5"
                                                            "\0\304\31\34\5\0\304\31\35\5\0\304\31\36\5\0\304\31\37\5\0\304\31 \5\0\304\31!\5\0\304"
                                                            "\31\42\5\0\304\31#\5\0\304\31$\5\0\304\31%\5\0\304\31&\5\0\304\31'\5\0\304\31("
                                                            "\5\0\304\31)\5\0\304\31*\5\0\304\31+\5\0\304\31,\5\0\304\31-\5\0\304\31.\6\42"
                                                            "\207\31\4/\5\0\304\31\60\15\210\204\71F\26\31\201\262\304\10\0\61\13\210\204y\3\211\25\35\23\4"
                                                            "\62\15\210\204\71F\224\15$T\210\30\4\63\12\210\204\31P\207\225:\1\64\12\210\204\31\203\314\64u"
                                                            "\4\65\10\210\204\31\222!K\66\10\210\204\31S\247I\67\10\210\204\31P\307\7\70\12\210\204\31\223\34"
                                                            "\31\232\0\71\10\210\204\31\322\246N:\5\0\304\31;\5\0\304\31<\5\0\304\31=\5\0\304\31>"
                                                            "\5\0\304\31\77\5\0\304\31@\5\0\304\31A\14\210\204\71F\26\31\262dh\0B\15\210\204\31G"
                                                            "\26)\31dH\12\0C\11\210\204\31S\307\12\1D\13\210\204\31G\26\231I\12\0E\13\210\204\31"
                                                            "S\15\31\32\12\1F\15\210\204\31S\207\32\62\64t(\0G\5\0\304\31H\13\210\204\31\203L\262"
                                                            "d\322\0I\12\210\204\31\320\4\213\15\2J\5\0\304\31K\5\0\304\31L\11\210\204\31\202\305\63\4"
                                                            "M\17\210\204\31\2\13\31\252\42\204\210\20\202\2N\16\210\204\31\303\14\35ar\310XA\1O\14\210"
                                                            "\204\71F\26\231i\211\21\0P\5\0\304\31Q\5\0\304\31R\16\210\204\31G\26\31Rrd\220\241"
                                                            "\1S\13\210\204\71\23\13\32\244\2\0T\5\0\304\31U\13\210\204\31\203\314i\212\21\0V\5\0\304"
                                                            "\31W\15\210\204\31\2\311\42\204\210\220J\3X\5\0\304\31Y\5\0\304\31Z\11\210\204\31\20\307\21"
                                                            "\2[\5\0\304\31\134\5\0\304\31]\5\0\304\31^\5\0\304\31_\5\0\304\31`\5\0\304\31a"
                                                            "\5\0\304\31b\5\0\304\31c\5\0\304\31d\5\0\304\31e\5\0\304\31f\5\0\304\31g\5\0"
                                                            "\304\31h\5\0\304\31i\5\0\304\31j\5\0\304\31k\5\0\304\31l\5\0\304\31m\5\0\304\31"
                                                            "n\5\0\304\31o\5\0\304\31p\5\0\304\31q\5\0\304\31r\5\0\304\31s\5\0\304\31t\5"
                                                            "\0\304\31u\5\0\304\31v\5\0\304\31w\5\0\304\31x\5\0\304\31y\5\0\304\31z\5\0\304"
                                                            "\31{\5\0\304\31|\5\0\304\31}\5\0\304\31~\5\0\304\31\177\5\0\304\31\0\0\0\4\377\377"
                                                            "\0";

void Display::init() {
    cursor_pos = 4;     // Default step size 1kHz.

    // Initialise display in full page buffer mode
    u8g2_SetupDisplay(&lcd, u8x8_d_ssd1309_128x64_noname0, u8x8_cad_ssd13xx_fast_i2c, u8x8_byte_rx231_hw_i2c, u8x8_gpio_and_delay_rx231);
    uint8_t tile_buf_height;
    uint8_t *buf = u8g2_m_16_8_1(&tile_buf_height);
    u8g2_SetupBuffer(&lcd, buf, tile_buf_height, u8g2_ll_hvline_vertical_top_lsb, U8G2_R0);
    R_BSP_SoftwareDelay(100, BSP_DELAY_MILLISECS);     // Wait for display to stabilise before sending any config instructions

    // Display config instructions
    u8g2_InitDisplay(&lcd);
    u8g2_SetPowerSave(&lcd, 1);
    u8g2_SetContrast(&lcd, 1);     // Lower contrast reduces OLED switching noise
    u8g2_ClearDisplay(&lcd);
    u8g2_SetPowerSave(&lcd, 0);  // enable display after RAM is clean
    u8g2_SetPowerSave(&lcd, 0);
    u8g2_SetFont(&lcd, sdr_font);

    // Adjust screen refresh rate
    u8x8_t *u8x8 = u8g2_GetU8x8(&lcd);
    u8x8_cad_StartTransfer(u8x8);
    u8x8_cad_SendCmd(u8x8, 0xD5);
    u8x8_cad_SendArg(u8x8, 0xF1);     // Rate adjustable
    u8x8_cad_EndTransfer(u8x8);
}

void Display::update(uint8_t *magnitudes) {
    u8g2_FirstPage(&lcd);
    do {
        u8g2_DrawHLine(&lcd, 0, 0, 128);
        u8g2_DrawHLine(&lcd, 0, 63, 128);
        u8g2_DrawVLine(&lcd, 0, 0, 64);
        u8g2_DrawVLine(&lcd, 127, 0, 64);
        draw_frequency();
        draw_cursor();
        draw_mode();
        draw_filter_width();
        draw_filter();
        draw_waterfall(magnitudes);
    } while (u8g2_NextPage(&lcd));
}

// String length helper.
size_t strlen(const char *s) {
    size_t count = 0;

    while (*s++) {
        count++;
    }
    return(count);
}

void Display::set_cursor_pos(int8_t rotations) {
    int8_t new_cursor_pos = (int8_t)cursor_pos + (int8_t)rotations;

    if (new_cursor_pos > MAX_CURSOR_POS) {
        cursor_pos = MAX_CURSOR_POS;
    } else if (new_cursor_pos < MIN_CURSOR_POS) {
        cursor_pos = MIN_CURSOR_POS;
    } else {
        cursor_pos = new_cursor_pos;
    }
}

uint8_t Display::get_cursor_pos() {
    return(cursor_pos);
}

void Display::draw_frequency() {
    u8g2_DrawStr(&lcd, FREQ_X_POS, FREQ_Y_POS, vfo.get_frequency_string());
    u8g2_DrawStr(&lcd, (128 - 1 - (3 * CHAR_WIDTH)), FREQ_Y_POS, "MHZ");
}

void Display::draw_mode() {
    u8g2_DrawStr(&lcd, MODE_X_POS, MODE_Y_POS, dsp.get_mode_string());
}

void Display::draw_filter_width() {
    uint8_t len = strlen(dsp.get_filter_width_string());

    u8g2_DrawStr(&lcd, FILTER_WIDTH_X_POS - (len * CHAR_WIDTH), FILTER_WIDTH_Y_POS, dsp.get_filter_width_string());
}

void Display::draw_filter() {
    uint8_t overall_length = dsp.get_filter_width_hz() / WATERFALL_RESOLUTION;

    // Keep filter centred - length must be even.
    if (overall_length % 2 != 0) {
        overall_length += 1;
    }

    // USB
    if (dsp.get_mode() == USB) {
        u8g2_DrawHLine(&lcd, FILTER_X_POS, FILTER_Y_POS, overall_length);
        u8g2_DrawVLine(&lcd, FILTER_X_POS, FILTER_Y_POS, FILTER_HEIGHT);
        u8g2_DrawVLine(&lcd, FILTER_X_POS + overall_length, FILTER_Y_POS, FILTER_HEIGHT);
    } else if (dsp.get_mode() == LSB) {
        u8g2_DrawHLine(&lcd, FILTER_X_POS - overall_length, FILTER_Y_POS, overall_length);
        u8g2_DrawVLine(&lcd, FILTER_X_POS, FILTER_Y_POS, FILTER_HEIGHT);
        u8g2_DrawVLine(&lcd, FILTER_X_POS - overall_length, FILTER_Y_POS, FILTER_HEIGHT);
    } else {
        u8g2_DrawHLine(&lcd, FILTER_X_POS - (overall_length / 2), FILTER_Y_POS, overall_length);
        u8g2_DrawVLine(&lcd, FILTER_X_POS - (overall_length / 2), FILTER_Y_POS, FILTER_HEIGHT);
        u8g2_DrawVLine(&lcd, FILTER_X_POS + (overall_length / 2), FILTER_Y_POS, FILTER_HEIGHT);
    }
}

void Display::draw_cursor() {
    if (cursor_pos <= 7) {
	// Draw cursor line under one character of frequency string
	// Frequency displayed as XX.XXX.XXX MHz
        uint8_t x_pos	= FREQ_X_POS + (cursor_pos * CHAR_WIDTH);
        uint8_t y_pos	= FREQ_Y_POS + 1;

	// Add offets to account for decimal points which are not counted as cursor positions
        if (cursor_pos > 1) {
            x_pos += CHAR_WIDTH;
        }

        if (cursor_pos > 4) {
            x_pos += CHAR_WIDTH;
        }

        u8g2_DrawHLine(&lcd, x_pos, y_pos, CHAR_WIDTH - 1);
    } else if (cursor_pos == 8) {
        // Draw cursor line under one entire mode string
        uint8_t x_pos	= MODE_X_POS;
        uint8_t y_pos	= MODE_Y_POS + 1;
        uint8_t len		= strlen(dsp.get_mode_string());
        u8g2_DrawHLine(&lcd, x_pos, y_pos, (CHAR_WIDTH * len) - 1);
    } else {
        // Draw cursor line under one entire filter width string
        uint8_t x_pos	= FILTER_WIDTH_X_POS;
        uint8_t y_pos	= FILTER_WIDTH_Y_POS + 1;
        uint8_t len		= strlen(dsp.get_filter_width_string());
        x_pos -= (len * CHAR_WIDTH);
        u8g2_DrawHLine(&lcd, x_pos, y_pos, (CHAR_WIDTH * len) - 1);
    }
}

void Display::draw_waterfall(uint8_t *magnitudes) {
    if (magnitudes != NULL) {
        for (uint8_t x = 0; x < 124; x++) {
            uint8_t bar_height = magnitudes[123 - x]; // Un-mirror waterfall spectrum casued by direct conversion IQ phasing
            if (bar_height > 32) {
                bar_height = 32;
            }
            if (bar_height > 0) {
                u8g2_DrawVLine(&lcd, x + 2, 62 - bar_height, bar_height);
            }
        }
    }
}
