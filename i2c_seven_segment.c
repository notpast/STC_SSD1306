

#include <stdint.h>

#include "ssd1306.h"
#include "stc_i2c.h"
#include "i2c_seven_segment.h"

#define seg_a    1
#define seg_b    2
#define seg_c    4
#define seg_d    8
#define seg_e   16
#define seg_f   32
#define seg_g   64
#define seg_dp 128




__code const uint8_t numbers_seg[18]= {
    seg_a|seg_b|seg_c|seg_d|seg_e|seg_f,        //0
    seg_b|seg_c,                                //1
    seg_a|seg_b|seg_g|seg_e|seg_d,              //2
    seg_a|seg_b|seg_c|seg_g|seg_d,              //3
    seg_c|seg_b|seg_g|seg_f,                    //4
    seg_a|seg_f|seg_g|seg_c|seg_d,              //5
    seg_a|seg_f|seg_g|seg_c|seg_d|seg_e,        //6
    seg_a|seg_b|seg_c,                          //7
    seg_a|seg_b|seg_c|seg_d|seg_e|seg_f|seg_g,  //8
    seg_a|seg_b|seg_c|seg_d|seg_f|seg_g,        //9
    seg_a|seg_b|seg_c|seg_e|seg_f|seg_g,        //A
    seg_c|seg_d|seg_e|seg_f|seg_g,              //B
    seg_a|seg_f|seg_e|seg_d,                    //C
    seg_b|seg_c|seg_d|seg_e|seg_g,              //D
    seg_a|seg_d|seg_e|seg_f|seg_g,              //E
    seg_a|seg_e|seg_f|seg_g,                    //F
    seg_g,                                      //Minus
    seg_dp                                      //Point
};




typedef struct {
    uint8_t min_x;
    uint8_t min_y;
    uint8_t max_x;
    uint8_t max_y;
} SEGMENT_SIZE;




#define LEFT_ARROW  3
#define RIGHT_ARROW 1
#define UP_ARROW    2
#define DOWN_ARROW  4

/*
    4 different arrow direction for (x,y)
    the result use to segments connection area

               _________ (len,len)
              |\       /|
              | \  4  / |
              |  \   /  |
              |   \ /   |
              | 1  \  3 |
              |   / \   |
              |  /   \  |
              | /  2  \ |
              |/       \|
        (0,0)  ---------

*/

uint8_t Arrow_Direction(uint8_t x,uint8_t y,uint8_t len) {

    if(x>=y) {
        if((len-x)>y) {
            return DOWN_ARROW;
        } else {
            return LEFT_ARROW;
        }
    } else {
        if(len-x>y) {
            return RIGHT_ARROW;
        } else {
            return UP_ARROW;
        }
    }
}




// timer: 827 clk
void Set_Segment_Size(SEGMENT_SIZE * segment_size,uint8_t size) {


    #define SEG_A 0
    #define SEG_B 1
    #define SEG_C 2
    #define SEG_D 3
    #define SEG_E 4
    #define SEG_F 5
    #define SEG_G 6
    #define SEG_DP 7


    // Segment thickness
    uint8_t seg_tkn=size/SEGMENT_THICKNESS_DIVISOR;

    // Min segment thickness is one
    if(seg_tkn<1) seg_tkn=1;

    uint8_t seg_width=size/2;
    uint8_t seg_height=size/2;

    // Only use for pozition of G segment
    uint8_t half_tkn=seg_tkn/2;


    /*

       A
     -----
   F|     |B
    |  G  |
     -----
   E|     |C
    |     |
     -----
       D

    */

    // Calculate bounding boxes for all segments at desired size

    //SEGMET A
    segment_size[SEG_A].min_x=0;
    segment_size[SEG_A].min_y=size-seg_tkn;
    segment_size[SEG_A].max_x=seg_width;
    segment_size[SEG_A].max_y=size;

    //SEGMET B
    segment_size[SEG_B].min_x=seg_width-seg_tkn;
    segment_size[SEG_B].min_y=seg_height;
    segment_size[SEG_B].max_x=seg_width;
    segment_size[SEG_B].max_y=size;

    //SEGMET C
    segment_size[SEG_C].min_x=seg_width-seg_tkn;
    segment_size[SEG_C].min_y=0;
    segment_size[SEG_C].max_x=seg_width;
    segment_size[SEG_C].max_y=seg_height;

    //SEGMET D
    segment_size[SEG_D].min_x=0;
    segment_size[SEG_D].min_y=0;
    segment_size[SEG_D].max_x=seg_width;
    segment_size[SEG_D].max_y=seg_tkn;

    //SEGMET E
    segment_size[SEG_E].min_x=0;
    segment_size[SEG_E].min_y=0;
    segment_size[SEG_E].max_x=seg_tkn;
    segment_size[SEG_E].max_y=seg_height;

    //SEGMET F
    segment_size[SEG_F].min_x=0;
    segment_size[SEG_F].min_y=seg_height;
    segment_size[SEG_F].max_x=seg_tkn;
    segment_size[SEG_F].max_y=size;

    //SEGMET G
    segment_size[SEG_G].min_x=0;
    segment_size[SEG_G].min_y=seg_height - half_tkn;
    segment_size[SEG_G].max_x=seg_width;
    segment_size[SEG_G].max_y=seg_height + seg_tkn - half_tkn;

    //SEGMET DP
    segment_size[SEG_DP].min_x=seg_width+1;
    segment_size[SEG_DP].min_y=0;
    segment_size[SEG_DP].max_x=seg_tkn+seg_width+2;
    segment_size[SEG_DP].max_y=seg_tkn;


}





// Check if the arrow belongs to the correct segment (timer: 74 clk)
uint8_t Check_Arrow_Direction(uint8_t segment, uint8_t arrow_direction,uint8_t arrow_boundary_x,uint8_t arrow_boundary_y) {

    // Arrow boudary box position total 6 segments connection area
    // X axis
    #define SEG_LEFT   1
    #define SEG_RIGHT  2

    // Y axis
    #define SEG_BOTTOM  1
    #define SEG_MIDLE   2
    #define SEG_TOP     3


    // Check left of the segments (bottom, middle, top segments)
    if(arrow_boundary_x==SEG_LEFT) {
        if(arrow_direction==LEFT_ARROW && (segment==SEG_A || segment==SEG_G || segment==SEG_D)) {
            return 1;
        }
        if(arrow_direction==UP_ARROW && ((segment==SEG_E && arrow_boundary_y==SEG_BOTTOM)   ||
                                         (segment==SEG_F && arrow_boundary_y==SEG_MIDLE))) {
            return 1;
        }
        if(arrow_direction==DOWN_ARROW && ((segment==SEG_E && arrow_boundary_y==SEG_MIDLE)   ||
                                           (segment==SEG_F && arrow_boundary_y==SEG_TOP))) {
            return 1;
        }
        // Check right of the segments (bottom, middle, top segments)
    } else if(arrow_boundary_x==SEG_RIGHT) {
        if(arrow_direction==RIGHT_ARROW && (segment==SEG_A || segment==SEG_G || segment==SEG_D)) {
            return 1;
        }
        if(arrow_direction==UP_ARROW && ((segment==SEG_C && arrow_boundary_y==SEG_BOTTOM)   ||
                                         (segment==SEG_B && arrow_boundary_y==SEG_MIDLE))) {
            return 1;
        }
        if(arrow_direction==DOWN_ARROW && ((segment==SEG_C && arrow_boundary_y==SEG_MIDLE)   ||
                                           (segment==SEG_B && arrow_boundary_y==SEG_TOP))) {
            return 1;
        }
    }
    return 0;
}



uint16_t Segment_Render(uint8_t px,uint8_t py,uint8_t size,uint8_t digit) {

    uint8_t seg=numbers_seg[digit&0x0F];

    // Check dp segment
    seg |=digit&0x80;

    uint8_t width =size/2;
    uint8_t t=0;

    uint8_t seg_tkn=size/SEGMENT_THICKNESS_DIVISOR;

    if(seg_tkn<1) seg_tkn=1;

    uint8_t half_tkn=seg_tkn/2;

    // Arrow boundary boxes x axis
    uint8_t min_x[2]= {0,width-seg_tkn};
    uint8_t max_x[2]= {seg_tkn,width};

    // Arrow boundary boxes y axis
    uint8_t min_y[3]= {0,width-half_tkn,size-seg_tkn};
    uint8_t max_y[3]= {seg_tkn,width + seg_tkn - half_tkn,size};

    // arrow_boundary_x : 3 different value if it's lef 1, right 2, if out of x boundary 0
    // arrow_boundary_y : 4 different value if it's bootom 1, middle 2, top 3, if out of y boundary 0
    uint8_t arrow_boundary_x,arrow_boundary_y;


    // Arrow direction 4 different values 1 to 4
    uint8_t arrow_direction;

    // Point x,y for subtract arrow area offset
    uint8_t arrow_x=0,arrow_y=0;


    SEGMENT_SIZE  segment_size[8];

    // Dynamic segment size calculation at runtime
    // Alternative: Precompute for all sizes (uses 32 bytes per target size)
    Set_Segment_Size(segment_size,size);

    for(uint8_t x=0; x<width + seg_tkn+2; x++) {
        uint8_t byte_pixel=0;
        t=0;

        // Check the boundary box of the arrows on the x-axis
        arrow_boundary_x=0;
        if(x>=min_x[0] && x<max_x[0]) {
            arrow_boundary_x=1;
            arrow_x=x;
        } else if(x>=min_x[1] && x<max_x[1]) {
            arrow_boundary_x=2;
            arrow_x=x-min_x[1];
        }

        for(uint8_t y=0; y<size; y++) {
            t++;
            byte_pixel=byte_pixel<<1;

            // Check the boundary box of the arrows on the y-axis
            arrow_boundary_y=0;


            if(arrow_boundary_x) {
                if(y>=min_y[0] && y<max_y[0]) {
                    arrow_boundary_y=1;
                    arrow_y=y;
                } else if(y>=min_y[1] && y<max_y[1]) {
                    arrow_boundary_y=2;
                    arrow_y=y-min_y[1];
                } else if(y>=min_y[2] && y<max_y[2]) {
                    arrow_boundary_y=3;
                    arrow_y=y-min_y[2];
                }
            }
            // All segments check for this point
            for(uint8_t i=0; i<8; i++) {

                // Check if include the segment
                if(((seg>>i)&0x01)==0) {
                    continue;
                }

                // Check if it is in the segment boundry
                if(segment_size[i].min_x<=x && segment_size[i].max_x>x) {
                    if(segment_size[i].min_y<=y && segment_size[i].max_y>y) {

                        // If size<16 don't check segment connection area (only draw segment as a rectangle)
                        if(size<16) {
                            byte_pixel|=1;
                            break;
                        }

                        // If point in the arrows boundary box
                        if(arrow_boundary_x && arrow_boundary_y) {

                            // Calculate arrow direction from desired point
                            arrow_direction=Arrow_Direction(arrow_x,arrow_y,seg_tkn);

                            // Check if the arrow belongs to the selected segment
                            if(Check_Arrow_Direction(i,arrow_direction,arrow_boundary_x,arrow_boundary_y)) {
                                byte_pixel|=1;
                                break;
                            }

                        } else {
                            byte_pixel|=1;
                            break;
                        }

                    }
                }
            }

            // Draw 8 pixels on the screen
            if(t==8) {
                SSD1306_Draw_Pixels(x+px,y+py,byte_pixel);
                byte_pixel=0;
                t=0;
            }
        }
        // Draw left over pixels
        if(t!=0) {
            // Black pixels complete to 8
            byte_pixel=byte_pixel<<(8-t);
            SSD1306_Draw_Pixels(x+px,(size)+py,byte_pixel);
        }
    }

    return 0;
}






