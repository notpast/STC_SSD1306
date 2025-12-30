
/*

    MIT License

    Copyright (c) 2025 Cetin Atila

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*/


#include <stdint.h>
#include <mcs51/at89x051.h>

#include "stc_i2c.h"
#include "ssd1306.h"
#include "ascii_font.h"
#include "i2c_seven_segment.h"
#include "stc8g_reg.h"



uint8_t Print_Number(uint8_t x,uint8_t y,uint8_t size,uint16_t number){
    uint8_t pos_x;

    // Calculate segment size
    uint8_t segment_width=size/2;
    uint8_t segment_thickness=size/SEGMENT_THICKNESS_DIVISOR;
    uint8_t space=4; // Space between digits
    uint8_t width;

    // Total width for per digit = (segment width) + (segment thickness for DP) + space
    width = segment_width + segment_thickness + space;

    // Print switch
    // 0: draw necessary digits
    // 1: draw 5 digits (Max 16 bit =65535 : 5 digits)
    uint8_t  print_sw =0;
    uint16_t div_dec  =10000;

    pos_x = x;

    for(uint8_t i=5;i>0;i--){
        uint8_t digit=(number/div_dec)%10;
        div_dec=div_dec/10;

        // If digit diffrent from 0
        // Blanks zeros to the left of the most significant digit
        if(digit) print_sw=1;

        if(print_sw){
             // Print one digit on the screen
             Segment_Render(pos_x,y,size,digit);
             pos_x+=width;
        }
    }

    // Return next caret x position
    return pos_x;
}






void main(void) {

    // Enable XDATA access
    P_SW2  = 0x80;

    CLKSEL = 0x00; // Select internal IRC (default 24Mhz)
    //CLKDIV = 0x01; // Clock divided value

    // I2C init; set pin configration and speed
    I2C_Init(400); // Speed 400Khz

    // SSD1306 display init
    SSD1306_Init();

    // Clear display
    SSD1306_Clear();


    uint8_t pos_x = 0;
    uint8_t pos_y = 0;
    uint8_t size  = 32;

    // Counter start with a number
    uint16_t counter = 27654;

    uint16_t limit= counter + 10;

    // Demo step 1
    while(counter<limit){
        // Print 5x8 text
        Print_Ascii_5x8(0,56," SEVEN-SEGMENT DEMO");
        Print_Ascii_5x8(0,48," ------------------");

        // Print seven-segment number
        Print_Number(pos_x,pos_y,size,counter);

        counter++;
    }


    // Demo step 2
    limit= counter + 150;
    // Clear display
    SSD1306_Clear();
    while(counter<limit){
        // Line 1
        Print_Number(0,0, 24,counter);
        // Line 2
        Print_Number(0,32,24,counter + 11111);
        counter++;
    }


    // Demo step 2
    limit= counter + 150;
    // Clear display
    SSD1306_Clear();
    while(counter<limit){
        // Line 1
        Print_Number(0,0,12,counter);
        // Line 2
        Print_Number(0,16,12,counter + 12345 );
        // Line 3
        Print_Number(0,32,12,counter + 11111);
        // Line 4
        Print_Number(0,48,12,counter + 33333);
        counter++;
    }


    // Demo step 3
    limit= counter + 50;
    // Clear display
    SSD1306_Clear();
    while(counter<limit){
        // Line 1
        Print_Number(0,0, 56, counter%100);
        // Line 2
        Print_Number(84,0,22, (counter+23)%100);
        // Line 3
        Print_Number(84,28,22,(counter+45)%100);
        counter++;
    }

    // Demo step 2
    counter=80;
    // Clear display
    SSD1306_Clear();
    while(counter<200){
        // Line 1
        Print_Number(0,0,64, counter%1000);
        counter++;
    }

}


