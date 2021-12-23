#include "display.h"
#include "gps.h"
#include <string>
#include <math.h>
/* Notes about display performance as of 12/20/21
TLDR: 
-Avoid calling sendBuffer() as much as possible, aim to do it only once per update cycle and outside of functions or classes
-Avoid clearing the buffer in performance cases as you then have to reconstruct the buffer which can incur a penlty
    if there are multiple fonts or functions that must be called in order to format the screen, 
    however this still seems to be relatively fast as the teensy is doing the work and no information is sent to the screen.
- use updateDisplayArea for 


Display performance seems decent in that it takes around 30ms to wite the screen black or white;
meaning every pixel in the buffer has been changed.
This also means every call to u8g2.sendBuffer() will take 30ms to complete assuming the buffer contains information that fills up the screen
or modifies pixels located in the corner of the screen as the full buffer needs to be sent to modify information in the last sections of display ram
if a small section needs to be changed quickly u8g2.updateDisplayArea() may be used, as it allows the ram to only be modified at the relevent bytes. 
The limititaion being that the x cordinates of the area update box cannot be modified (with the 6963 display)
this may be a limitation of the u8g2 library, but the controller datasheet does not seem to allow selection of columns for a start.
also note that the function requires tile cordinates which are 8x8 sections of the display. Also the display has been rotated 180*
for use in J-box. So a translation must be done to arrive at the appropriate tile cordinates for an update.

also note that amount of time to write information to the display likely cannot be sped up as the controller datasheet
requires the CE line to be off for 80ns with the data on the ports then on for another 40ns with the data still on the bus.
this means 120ns per byte of data written to the screen, 
if update times are affecting other actions consider implementing timed interupts in u8g2

*/
int screenx = 240;
int screeny = 128;
int fontx = 12;
int fonty = 15;
#define JboxIcon_width 100
#define JboxIcon_height 100
static unsigned char JboxIcon_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x07,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdc, 0x00, 0x00, 0x00, 0x80,
   0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x77, 0x00, 0x00, 0x00,
   0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x00,
   0x00, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x8f, 0xcf, 0x00,
   0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x67,
   0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0,
   0xf2, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xe0, 0xd3, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x20, 0xe9, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x20, 0x7d, 0x00, 0x00, 0x00, 0x60, 0x60, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xdf, 0xa0, 0xdf, 0x00, 0x00, 0x00, 0x60, 0xe0, 0xff, 0xff,
   0xff, 0xff, 0xff, 0x1f, 0xe1, 0xce, 0x00, 0x00, 0x00, 0x60, 0x20, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x80, 0xe1, 0xea, 0x00, 0x00, 0x00, 0x60, 0x20,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xa1, 0xd7, 0x00, 0x00, 0x00, 0x60,
   0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xa1, 0xea, 0x00, 0x00, 0x00,
   0x60, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xa1, 0xf6, 0x00, 0x00,
   0x00, 0x60, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xa1, 0xdf, 0x00,
   0x00, 0x00, 0x60, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xe1, 0xfb,
   0x00, 0x00, 0x00, 0x60, 0x20, 0x00, 0x00, 0xe0, 0x01, 0x00, 0x80, 0xe1,
   0xed, 0x00, 0x00, 0x00, 0x60, 0x20, 0x00, 0x00, 0x60, 0x01, 0x00, 0x80,
   0xe1, 0xf7, 0x00, 0x00, 0x00, 0x60, 0x20, 0x00, 0x00, 0xe0, 0x01, 0x00,
   0x80, 0xe1, 0x7d, 0x00, 0x00, 0x00, 0x60, 0x20, 0x00, 0x00, 0xe0, 0x01,
   0x00, 0x80, 0xa1, 0x6e, 0x00, 0x00, 0x00, 0x60, 0x20, 0x00, 0x00, 0xe0,
   0x00, 0x00, 0x80, 0xa1, 0xc7, 0x00, 0x00, 0x00, 0x60, 0x20, 0x00, 0x00,
   0xe0, 0x01, 0x00, 0x80, 0xa1, 0xd3, 0x00, 0x00, 0x00, 0x60, 0x20, 0x00,
   0x00, 0xe0, 0x01, 0x00, 0x80, 0xa1, 0xda, 0x00, 0x00, 0x00, 0x60, 0x20,
   0x00, 0x00, 0xe0, 0x01, 0x00, 0x80, 0xa1, 0xbd, 0x00, 0x00, 0x00, 0x60,
   0x20, 0x00, 0x00, 0xa0, 0x01, 0x00, 0x80, 0xa1, 0x97, 0x00, 0x00, 0x00,
   0x60, 0x20, 0x00, 0x00, 0xa0, 0x01, 0x00, 0x80, 0xa1, 0x8d, 0x00, 0x00,
   0x00, 0x60, 0x20, 0x00, 0x00, 0x60, 0x01, 0x00, 0x80, 0xe1, 0xee, 0x00,
   0x00, 0x00, 0x60, 0x20, 0x00, 0x00, 0x60, 0x01, 0x00, 0x80, 0xe1, 0xb6,
   0x00, 0x00, 0x00, 0x60, 0x20, 0x00, 0x00, 0xe0, 0x01, 0x00, 0x80, 0xe1,
   0xfe, 0x00, 0x00, 0x00, 0x60, 0x20, 0x00, 0x00, 0xa0, 0x01, 0x00, 0x80,
   0xa1, 0xea, 0x00, 0x00, 0x00, 0x60, 0x20, 0x00, 0x1c, 0xa0, 0x01, 0x00,
   0x80, 0xa1, 0xad, 0x00, 0x00, 0x00, 0x60, 0x20, 0x00, 0x18, 0xe0, 0x01,
   0x00, 0x80, 0xa1, 0xe7, 0x00, 0x00, 0x00, 0x60, 0x20, 0x00, 0x1c, 0x60,
   0x01, 0x00, 0x80, 0xa1, 0xf3, 0x00, 0x00, 0x00, 0x60, 0x20, 0x00, 0x1c,
   0x60, 0x01, 0x00, 0x80, 0xa1, 0xea, 0x00, 0x00, 0x00, 0x60, 0x20, 0x00,
   0x24, 0xf0, 0x01, 0x00, 0x80, 0xa1, 0xcd, 0x00, 0x00, 0x00, 0x60, 0x20,
   0x00, 0xe8, 0xff, 0x00, 0x00, 0x80, 0xa1, 0xd6, 0x00, 0x00, 0x00, 0x60,
   0x20, 0x00, 0x70, 0x7f, 0x00, 0x00, 0x80, 0xe1, 0xcb, 0x00, 0x00, 0x00,
   0x60, 0x20, 0x00, 0x80, 0x1f, 0x00, 0x00, 0x80, 0xa1, 0xaa, 0x00, 0x00,
   0x00, 0x60, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xa1, 0x95, 0x00,
   0x00, 0x00, 0x60, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xe1, 0xba,
   0x00, 0x00, 0x00, 0x60, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xe1,
   0xaf, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0xe1, 0xd7, 0x00, 0x00, 0x00, 0x60, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x80, 0xa1, 0xad, 0x00, 0x00, 0x00, 0x60, 0x20, 0x10, 0x02, 0x00, 0x00,
   0x3c, 0xf1, 0x61, 0xd6, 0x00, 0x00, 0x00, 0x60, 0xe0, 0xff, 0xff, 0xff,
   0xff, 0xff, 0x7f, 0xe0, 0xf7, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xa0, 0x7b, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0xff, 0x00, 0x00, 0x00, 0x60, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0xef, 0xde, 0x00, 0x00, 0x60,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x36, 0xc0, 0x00, 0x00,
   0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x0e, 0xe0, 0x00,
   0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0e, 0xb8,
   0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x05,
   0xfc, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
   0x01, 0xd6, 0x00, 0x00, 0xe8, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xff,
   0xbf, 0x00, 0xde, 0x00, 0x00, 0xe4, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff,
   0xff, 0x7e, 0x80, 0xff, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xc0, 0xd7, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xe0, 0xd3, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0xf7, 0x00, 0xe0, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xfd, 0x00, 0x60, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xdf, 0x00, 0x60, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0xf7, 0x00, 0x60,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xad, 0x00,
   0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff,
   0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x98,
   0xd7, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xf8, 0x6e, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xf8, 0xff, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xf8, 0xf6, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xd8, 0xd3, 0x00, 0x60, 0x80, 0x0f, 0x00, 0x00, 0x00,
   0xfb, 0xef, 0xff, 0x07, 0xf8, 0xff, 0x00, 0x60, 0x80, 0x0e, 0x00, 0x00,
   0x00, 0xff, 0xff, 0xff, 0x07, 0x38, 0xbd, 0x00, 0x60, 0x80, 0x0f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd8, 0xff, 0x00, 0x60, 0x80, 0x09,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x77, 0x00, 0x60, 0x80,
   0x0f, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x3b, 0x00, 0x60,
   0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x1d, 0x00,
   0x60, 0x00, 0x00, 0x0c, 0x70, 0xf8, 0x98, 0x01, 0x00, 0x00, 0xd8, 0x0f,
   0x00, 0x60, 0x00, 0x00, 0xcc, 0x71, 0x88, 0xd0, 0x00, 0x00, 0x00, 0x38,
   0x07, 0x00, 0x60, 0x00, 0x60, 0x0c, 0x90, 0xa8, 0x60, 0x00, 0x00, 0x00,
   0xf8, 0x03, 0x00, 0x60, 0x00, 0x60, 0x0c, 0x90, 0x88, 0xb0, 0x00, 0x00,
   0x00, 0xf8, 0x01, 0x00, 0x60, 0x00, 0xe0, 0x0f, 0xf0, 0xf8, 0x98, 0x01,
   0x00, 0x00, 0xd8, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00 };
#define ScRacing_width 63
#define ScRacing_height 72
static unsigned char ScRacing_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c,
   0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x03, 0x1c, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x60, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00,
   0x80, 0x03, 0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0x1c, 0x00, 0x00,
   0x00, 0x7c, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0xf0, 0x03, 0x00, 0x00,
   0x00, 0x00, 0xfc, 0x00, 0x10, 0x00, 0x00, 0xf0, 0x01, 0x00, 0x80, 0x00,
   0x10, 0x00, 0x00, 0xfe, 0xff, 0x00, 0x80, 0x01, 0x10, 0x00, 0xc0, 0xff,
   0xff, 0x0f, 0x80, 0x01, 0x10, 0x00, 0xe0, 0xff, 0xff, 0x07, 0x80, 0x01,
   0x10, 0x00, 0xf0, 0xff, 0xff, 0x01, 0x80, 0x01, 0x10, 0x00, 0xf8, 0xff,
   0xff, 0x01, 0x80, 0x01, 0x10, 0x00, 0xfc, 0xff, 0xff, 0x00, 0x80, 0x00,
   0x10, 0x00, 0xfc, 0x1f, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0xfc, 0x07,
   0x0e, 0x00, 0x80, 0x00, 0x10, 0x00, 0xfe, 0xe1, 0xff, 0x00, 0x80, 0x00,
   0x10, 0x00, 0xfe, 0xf8, 0xff, 0x03, 0x80, 0x00, 0x10, 0x00, 0x7e, 0xfc,
   0xff, 0x07, 0x80, 0x00, 0x10, 0x00, 0x7e, 0xfe, 0xff, 0x0f, 0x80, 0x00,
   0x10, 0x00, 0x3e, 0xfe, 0xff, 0x0f, 0x80, 0x00, 0x10, 0x00, 0x3e, 0xff,
   0xff, 0x00, 0x80, 0x00, 0x10, 0x00, 0x3e, 0xff, 0x1f, 0x00, 0x80, 0x00,
   0x10, 0x00, 0xbe, 0xff, 0x0f, 0x00, 0x80, 0x00, 0x10, 0x00, 0xbe, 0xff,
   0x07, 0x00, 0x80, 0x00, 0x10, 0x00, 0x9e, 0xff, 0x07, 0x00, 0x80, 0x00,
   0x10, 0x00, 0x9e, 0xff, 0x0f, 0x00, 0x80, 0x00, 0x20, 0x00, 0x9e, 0xff,
   0x3f, 0x78, 0x80, 0x00, 0x20, 0x00, 0x9e, 0xff, 0xff, 0xff, 0xc0, 0x00,
   0x20, 0x00, 0x9f, 0xff, 0xff, 0xff, 0x40, 0x00, 0x20, 0x00, 0x0f, 0xff,
   0xff, 0xff, 0x40, 0x00, 0x20, 0x00, 0x0f, 0xff, 0xff, 0xff, 0x60, 0x00,
   0x40, 0x00, 0x07, 0xff, 0xff, 0xff, 0x20, 0x00, 0x40, 0x80, 0x03, 0xfe,
   0xff, 0xff, 0x20, 0x00, 0x40, 0xc0, 0x01, 0x00, 0x00, 0xe0, 0x30, 0x00,
   0x80, 0x60, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x10, 0x00, 0x00,
   0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00,
   0x00, 0x01, 0x82, 0xfd, 0xf3, 0x0f, 0x08, 0x00, 0x00, 0x01, 0x82, 0x0d,
   0x10, 0x00, 0x08, 0x00, 0x00, 0x03, 0x82, 0xfd, 0x13, 0x00, 0x0c, 0x00,
   0x00, 0x02, 0x82, 0xfd, 0x13, 0x00, 0x04, 0x00, 0x00, 0x06, 0x82, 0x01,
   0x12, 0x00, 0x06, 0x00, 0x00, 0x04, 0xfe, 0xfd, 0xf3, 0x0f, 0x02, 0x00,
   0x00, 0x0c, 0xfe, 0xfd, 0xf3, 0x0f, 0x02, 0x00, 0x00, 0x08, 0x00, 0x00,
   0x00, 0x00, 0x03, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
   0x00, 0x30, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x60, 0x00, 0x00,
   0x00, 0xc0, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00,
   0x00, 0xc0, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00,
   0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x18, 0x00, 0x00,
   0x00, 0x00, 0x07, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00,
   0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x03, 0x00, 0x00,
   0x00, 0x00, 0x38, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00,
   0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x60, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x80, 0x01, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
   0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x0c, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x08, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0,
   0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };



unsigned long lastMphUpdate;
unsigned long lastRpmUpdate;
unsigned long lastCTempUpdate;
unsigned long lastETempUpdate;
unsigned long lastVoltUpdate;
unsigned long lastFuelUpdate;
unsigned long lastLapTimeUpdate;
unsigned long lapTimeStart;
unsigned long lastGearUpdate;
// unsigned long lastTimeUpdate;
int mph=0;
int rpm=0;
int cutoff = 2000;
int lapTime[3] = {0,0,0};
int coolantTemp = 90;
int engineTemp = 90;
int fuelLevel = 100;
double battVoltage = 11.5;
String gears[6] = {"N","1","2","3","4","5"}; // this could probably be done with just a char [] (with less resources)
int gIndex = 0;
int gearDir = 1;



U8G2_T6963_240X128_F_8080 u8g2(U8G2_R2, 2, 14, 7, 8, 6, 20, 21, 5, /*enable/wr=*/ 27 , /*cs/ce=*/ 26, /*dc=*/ 25, /*reset=*/24); // Connect RD (orange) with +5V, FS0 and FS1 with GND
TSPoint p;
void modifyPointToScreen()
{
p.x =  map(p.x,100,944,0,240);
p.y =  map(p.y,190,860,0,128);
p.z = abs(map(p.z,900,300,0,255));
}
TouchScreen ts = TouchScreen(XP, YP, XM, YM , 730);
//enum screens {menu,settings,gps,naughtTo60Timer};
//screens currentScreen = settings;

void initializeDisplay()
{
    pinMode(backlightPin,OUTPUT);
    analogWrite(backlightPin,100);
    u8g2.begin();
    //u8g2.setFlipMode(1); //6963 does not seem to support rotation of internal buffer
    u8g2.setContrast(255);
    u8g2.clearBuffer();
    // u8g2.setFont(u8g2_font_logisoso58_tf);
    // u8g2.drawStr(25,80,"J-Box");
    u8g2.drawXBM( 80, 10, JboxIcon_width, JboxIcon_height, JboxIcon_bits);
    u8g2.drawXBM( 0, 30, ScRacing_width, ScRacing_height, ScRacing_bits);
    u8g2.setFont(u8g2_font_courR10_tr);
    u8g2.setFontPosBaseline(); // Set the font position to the bottom
    // u8g2.drawStr(160,10,"J-Box");
    // u8g2.drawStr(160,23,"Bootup");
    u8g2.sendBuffer();
    delay(2000);
    u8g2.clearBuffer();
}
void initializeBakerFSAEscreen()
{
  lastMphUpdate = 0;
  lastRpmUpdate = 0;
  lastCTempUpdate = 0;
  lastETempUpdate = 0;
  lastVoltUpdate = 0;
  lastFuelUpdate = 0;
  lastLapTimeUpdate = 0;
  lastGearUpdate = 0;
  lapTimeStart = millis();
  drawBackground2();
  drawBoxGauge(rpm, 12000, 4000, 10000);
  drawGear(gears[gIndex]);
  drawCoolantTemp(coolantTemp);
  drawEngineTemp(engineTemp);
  drawFuel(fuelLevel);
  drawVoltage(battVoltage);  
}
void BakerFSAEscreen()
{
    
//   if (rpm==8000) {
//     rpm = 0;
//   } else {
//     rpm+=100;    
//   }
  //drawBoxGauge(rpm, 8500,cutoff);
  
  if (millis()-lastMphUpdate>500){
    if (mph==99) {
      mph = 0;
    } else {
      mph++;
    }
    drawMph(mph);
    lastMphUpdate = millis();
  }

  if (millis()-lastLapTimeUpdate>=1000){
    int seconds = millis()/1000;
    lapTime[2] = seconds%60;
    lapTime[1] = (seconds/60)%60;
    lapTime[0] = seconds/3600;
    drawLapTime(lapTime);
    lastLapTimeUpdate = millis();
  }

  if (millis()-lastFuelUpdate>3000){
    if (fuelLevel==0) {
      fuelLevel = 100;
    } else {
      fuelLevel--;
    }
    drawFuel(fuelLevel);
    lastFuelUpdate = millis();
  }

  if (millis()-lastVoltUpdate>1000){
    if (battVoltage<10.0) {
      battVoltage = 12.0;
    } else {
      battVoltage=battVoltage-.1;
    }
    drawVoltage(battVoltage);
    lastVoltUpdate = millis();
  }

  if (millis()-lastCTempUpdate>4000){
    if (coolantTemp==120) {
      coolantTemp = 90;
    } else {
      coolantTemp++;
    }
    drawCoolantTemp(coolantTemp);
    lastCTempUpdate = millis();
  }

  if (millis()-lastETempUpdate>4500){
    if (engineTemp==120) {
      engineTemp = 90;
    } else {
      engineTemp++;
    }
    drawEngineTemp(coolantTemp);
    lastETempUpdate = millis();
  }

  if (millis()-lastGearUpdate>6000){
    if (gIndex<=0) {
      gearDir = 1;
    } else if (gIndex>=5) {
      gearDir = -1;
    }
    drawGear(gears[gIndex]);
    gIndex+=gearDir;
    lastGearUpdate = millis();
  }
}

void initializeEaganInsightScreen()
{
    //drawBoxGauge(rpm, 6500, 1000, 5800);
}
void menuScreen()
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_VCR_OSD_mf); //u8g2_font_9x15B_mr
    u8g2.drawStr(32,18,"Menu");
    u8g2.drawLine(0,19,240,19);
    ///u8g2.setFontMode(0); // shoulnt need to change this as it is the default
    u8g2.setDrawColor(0); // set this to zero so that we get a black background against the font
    u8g2.drawStr(0,40,"Device Settings");
    u8g2.drawStr(0,60,"");
    u8g2.drawStr(0,80,"Acceleration Tests");
    

    
    
    
    u8g2.sendBuffer();
    u8g2.setDrawColor(1); // reset the draw color back to the default




}

// Draws a portion of a cicle, angle in radians. Keep in mind y axis is flipped.
void drawPartialCircle(int xo, int yo, double radius, double initialAngle, double finalAngle){
    for (double angle = initialAngle; angle<=finalAngle; angle = angle + .01){
        u8g2.drawPixel(xo+int(radius*cos(angle)),yo+int(radius*sin(angle)));
    }
}

//Draws a circular gauge with a "needle" at some proportion of a maximum
void drawGauge(int xo, int yo, int rad, double maxValue, double currentValue) {
    double initAngle = M_PI*(1.0/4.0);
    double finalAngle = M_PI*(7.0/4.0);
    drawPartialCircle(xo,yo,rad*1.0,initAngle,finalAngle);
    double angle = (currentValue/maxValue)*(finalAngle-initAngle)+initAngle;
    //needle 3 pixels thick
    for (int i = -1; i<=1; i++) {
        for (int j = -1; j<=1; j++) {
            u8g2.drawLine(xo+i,yo+j,xo+int(rad*cos(angle))+i,yo+int(rad*sin(angle))+j);
        }
    }
    // u8g2.drawLine(xo-1,yo-1,xo+int(rad*cos(angle))-1,yo+int(rad*sin(angle))-1);
    // u8g2.drawLine(xo,yo,xo+int(rad*cos(angle)),yo+int(rad*sin(angle)));
    // u8g2.drawLine(xo+1,yo+1,xo+int(rad*cos(angle))+1,yo+int(rad*sin(angle))+1);
    //markers and labels
    u8g2.setFont(u8g2_font_chroma48medium8_8n);
    int i = 0;
    int numNotches = maxValue/1000;
    for (double angle = initAngle; angle<=finalAngle; angle = angle + (finalAngle-initAngle)/numNotches){
        u8g2.drawLine(xo+int(rad*.85*cos(angle)),yo+int(rad*.85*sin(angle)),xo+int(rad*cos(angle)),yo+int(rad*sin(angle)));
        u8g2.drawStr(xo+int(rad*.85*cos(angle)-8.0/sqrt(2)),yo+int(rad*.85*sin(angle)+8.0/sqrt(2)),String(i).c_str());
        i = i+1;
    }
}

void drawCircularBarGauge(int xo, int yo, int rad, double maxValue, double currentValue) {
    double initAngle = M_PI*(1.0/4.0);
    double finalAngle = M_PI*(7.0/4.0);
    double currentAngle = (currentValue/maxValue)*(finalAngle-initAngle)+initAngle;
    for (double angle = initAngle; angle<=currentAngle; angle = angle + .001){
        double lin = (currentValue / maxValue) / 2.0;
        u8g2.drawLine(xo+int(rad*lin*cos(angle)),yo+int(rad*lin*sin(angle)),xo+int(rad*cos(angle)),yo+int(rad*sin(angle)));
    }
}

void clearBox(int x0, int y0, int w, int h) {
    //TODO: replace all appearances of the below with this method
    u8g2.setDrawColor(0);
    u8g2.drawBox(x0,y0,w,h);
    u8g2.setDrawColor(1);
}

void drawGear(String gear){
    int w = 45;
    int h = 68;
    int x0 = screenx/2-w/2;
    int y0 = screeny/2-21;
    clearBox(x0,y0,w,h);
    u8g2.setFont(u8g2_font_logisoso58_tf);
    u8g2.drawStr(x0+4,y0+63,gear.c_str());
    u8g2.drawFrame(x0,y0,w,h);
}

void drawMph(int mph) {
    int x0 = 12;
    int y0 = screeny/2+20;
    clearBox(x0,y0-30,40,31);
    u8g2.setFont(u8g2_font_logisoso30_tr);
    if (mph<10) {
        u8g2.drawStr(x0+20,y0,String(mph).c_str());
    } else {
        u8g2.drawStr(x0,y0,String(mph).c_str());
    }
    u8g2.sendBuffer();
}

void drawRpm(int rpm) {
    u8g2.setDrawColor(0);
    u8g2.drawBox(110,0,150,31);
    u8g2.drawBox(150,0,fontx*3,fonty*2+1);
    u8g2.drawDisc(screenx-55,screeny/2,46);
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_logisoso30_tr);
    if (rpm<10000) {
        u8g2.drawStr(130,30,String(rpm/1000).c_str());
    } else {
        u8g2.drawStr(110,30,String(rpm/1000).c_str());
    }
    u8g2.setFont(u8g2_font_VCR_OSD_mf);
    u8g2.drawStr(150,fonty,String(rpm%1000).c_str());
    drawGauge(screenx-55,screeny/2,45,12000.0,rpm*1.0);
    u8g2.sendBuffer();
}

void drawCoolantTemp(int coolantTemp) {
    int x0 = (coolantTemp>=100) ? 0 : fontx;
    int y0 = screeny-fonty;
    clearBox(x0,y0-fonty,fontx*(2+(coolantTemp/100)),fonty);
    u8g2.setFont(u8g2_font_VCR_OSD_mf);
    u8g2.drawStr(x0,y0,String(coolantTemp).c_str());
}
void drawEngineTemp(int engineTemp) {
    int x0 = (engineTemp>=100) ? 0 : fontx;
    int y0 = screeny;
    clearBox(x0,y0-fonty,fontx*(2+(engineTemp/100)),fonty);
    u8g2.setFont(u8g2_font_VCR_OSD_mf);
    u8g2.drawStr(x0,y0,String(engineTemp).c_str());
}

void drawFuel(int fuelLevel) {
    u8g2.setFont(u8g2_font_VCR_OSD_mf);
    int x0 = (screenx/2)-fontx*(9/2);
    int y0 = screeny;
    clearBox(x0,y0-fonty,fontx*3,fonty);
    const char *fuelStr = String(fuelLevel).c_str();
    if (fuelLevel>=100){
        u8g2.drawStr(x0,y0,fuelStr);
    } else if (fuelLevel>10) {
        u8g2.drawStr(x0+fontx,y0,fuelStr);
    } else {
        u8g2.drawStr(x0+2*fontx,y0,fuelStr);
    }
}

void drawLapTime(int *lapTime){
    u8g2.setFont(u8g2_font_VCR_OSD_mf);
    for (int i=0;i<3;i++) {
        clearBox(screenx-fontx*8 +fontx*i*3,screeny-fonty,fontx*2,fonty);
    }
    if (lapTime[0]<10) {
        u8g2.drawStr(screenx-fontx*7,screeny,String(lapTime[0]).c_str());
    } else {
        u8g2.drawStr(screenx-fontx*8,screeny,String(lapTime[0]).c_str());
    }
    if (lapTime[1]<10) {
        u8g2.drawStr(screenx-fontx*5,screeny,"0");
        u8g2.drawStr(screenx-fontx*4,screeny,String(lapTime[1]).c_str());
    }
    else {
        u8g2.drawStr(screenx-fontx*5,screeny,String(lapTime[1]).c_str());
    }
    if (lapTime[2]<10) {
        u8g2.drawStr(screenx-fontx*2,screeny,"0");
        u8g2.drawStr(screenx-fontx*1,screeny,String(lapTime[2]).c_str());
    }
    else {
        u8g2.drawStr(screenx-fontx*2,screeny,String(lapTime[2]).c_str());
    }
}

void drawVoltage(double battVoltage) {
    u8g2.setFont(u8g2_font_VCR_OSD_mf);
    int x0 = screenx-fontx*5 + fontx/2;
    int y0 = screeny-fonty;
    clearBox(x0,y0-fonty,fontx*4 - fontx/2, fonty);
    u8g2.setFontMode(1);
    int tensOffset = (battVoltage<10) ? fontx : 0;
    u8g2.drawStr(x0+tensOffset,y0,String(int(battVoltage)).c_str());
    int firstDecimal = int(battVoltage*10.0) % 10;
    u8g2.drawStr(x0+fontx*3-fontx/2,y0,String(firstDecimal).c_str());
    u8g2.drawStr(x0+fontx*2-fontx/4,y0,".");
    u8g2.setFontMode(0);

}

void drawBackground() {
    int screenx = 240;
    int screeny = 128;
    u8g2.setFont(u8g2_font_VCR_OSD_mf);
    int fontx = 12;
    int fonty = 15;

    u8g2.drawFrame(screenx/3-4,screeny/2-32,46,68);
    
    u8g2.drawStr(40,fonty,"mph");
    
    u8g2.drawStr(screenx - fontx*3,fonty,"rpm");

    u8g2.drawStr(fontx*3,screeny-fonty,"C");

    u8g2.drawStr(fontx*3,screeny,"C");
    
    u8g2.drawStr((screenx/2),screeny,"%");

    u8g2.drawStr(screenx-fontx*6,screeny,":");

    u8g2.drawStr(screenx-fontx*3,screeny,":");

    u8g2.drawStr(fontx*4,screeny/2+fonty/2,"V");
}

void circularGaugeLayout() {
    int mph = 69;
    String gear = "3";
    int rpm = 11420;
    int coolantTemp = 109;
    int engineTemp = 120;
    int fuelLevel = 42;
    int lapTime[3] = {1,23,45};
    double battVoltage = 11.572345;
    

    u8g2.clearBuffer();
    
    drawBackground();

    
    drawGear(gear);
    drawMph(mph);
    drawRpm(rpm);
    drawCoolantTemp(coolantTemp);
    drawEngineTemp(engineTemp);
    drawFuel(fuelLevel);
    drawLapTime(lapTime);
    drawVoltage(battVoltage);
    u8g2.sendBuffer();
}

void drawBackground2() {

    int screenx = 240;
    int screeny = 128;
    u8g2.setFont(u8g2_font_VCR_OSD_mf);
    int fontx = 12;
    int fonty = 15;
    
    u8g2.drawStr(52,screeny/2+20,"mph");
    
    u8g2.drawStr(fontx*3,screeny-fonty,"C");

    u8g2.drawStr(fontx*3,screeny,"C");
    
    u8g2.drawStr((screenx/2)-fontx,screeny,"%");

    u8g2.drawStr(screenx-fontx*6,screeny,":");

    u8g2.drawStr(screenx-fontx*3,screeny,":");

    u8g2.drawStr(screenx-fontx,screeny-fonty,"V");

}

/* void drawBoxGauge(int current, int max, int cutoff = 4000, int redLine = 10000) {
    int padding = 2;
    int xStart = padding;
    int yStart = padding;
    int width = screenx-2*padding-40;
    int height = screeny/5;

    u8g2.setDrawColor(0);
    u8g2.drawBox(xStart,yStart,screenx-xStart,height);
    u8g2.setDrawColor(1);
    u8g2.drawFrame(xStart,yStart,width,height);

    int cutoffWidth = int(width*(float(cutoff/2.0)/float(max)));

    if (current <= cutoff) {
        u8g2.drawBox(xStart,yStart,int(width*(float(current/2.0)/float(max))),height);
    } else {
        u8g2.drawBox(xStart,yStart,cutoffWidth,height);
        u8g2.drawBox(xStart+cutoffWidth,yStart,int((width-cutoffWidth)*(float(current - cutoff)/float(max - cutoff))),height);
    }
    for (int i = 0; i<=max; i+=1000) {
        int offset;
        if (i<=cutoff) {
            offset=int((float(i)/float(max))*width)/2;
        } else {
            offset=cutoffWidth+int((float(i-cutoff)/float(max-cutoff))*(width-cutoffWidth));
        }
        if (i==redLine) {
            u8g2.drawVLine(xStart + offset, yStart, 5+height);
        } else {
            u8g2.drawVLine(xStart + offset, yStart+height, 5);
        }
        if (i%2000!=0 && i<=cutoff) {
            continue;
        }
        u8g2.setFont(u8g2_font_bitcasual_tn);
        int xfontoff = (i<10000) ? 2 : 5;
        u8g2.drawStr(xStart + offset - xfontoff, yStart+height+5+8,String(i/1000).c_str());
    }
    u8g2.setFont(u8g2_font_logisoso18_tf);
    int newFontx = 11;
    int yOff = 2;
    int xOff = 2;
    if (current<redLine) {
        clearBox(screenx/2+25,screeny/2+29-32,screenx/2,33);
        u8g2.drawStr(xStart+width+xOff,yStart+height-yOff,"0");
        u8g2.drawStr(xStart+width+newFontx+xOff,yStart+height-yOff,String(current/1000).c_str());
    } else {
        u8g2.drawStr(xStart+width+xOff,yStart+height-yOff,String(current/1000).c_str());
        u8g2.setFont(u8g2_font_logisoso32_tf);
        u8g2.drawStr(screenx/2+25,screeny/2+29,"SHIFT");
        u8g2.setFont(u8g2_font_logisoso18_tf);
    }
    u8g2.drawStr(xStart+width+2*newFontx+xOff,yStart+height-yOff,".");
    u8g2.drawStr(xStart+width+3*newFontx+xOff-6,yStart+height-yOff,String((current%1000)/100).c_str());
    u8g2.sendBuffer();
}
*/

class boxGauge
    {
    public:
    int padding = 2;
    int xStart = padding;
    int yStart = padding;
    int width = screenx-2*padding-40;
    int height = screeny/5;
    int max = 12000; int cutoff = 4000; int redLine = 10000;
    bool shiftText = false;

    private: // hidden variables

    public: // functions

    void display(int current){ 
        // this will not clear the box as is
     clearBox(xStart,yStart,screenx-xStart,height);
     u8g2.drawFrame(xStart,yStart,width,height);
     int cutoffWidth = int(width*(float(cutoff/2.0)/float(max)));
     if (current <= cutoff) { // this part draws and clears the filled in bars
        u8g2.drawBox(xStart,yStart,int(width*(float(current/2.0)/float(max))),height);
        } else {
        u8g2.drawBox(xStart,yStart,cutoffWidth,height);
        u8g2.drawBox(xStart+cutoffWidth,yStart,int((width-cutoffWidth)*(float(current - cutoff)/float(max - cutoff))),height);
        } 
    }

    void drawBoxGauge(int current) { // idea: numbers that exist within the boxgauge showing the current value / shift
    clearBox(xStart,yStart,screenx-xStart,height);
    u8g2.drawFrame(xStart,yStart,width,height);
    int cutoffWidth = int(width*(float(cutoff/2.0)/float(max)));
    if (current <= cutoff) { // this part draws and clears the filled in bars
        u8g2.drawBox(xStart,yStart,int(width*(float(current/2.0)/float(max))),height);
    } else {
        u8g2.drawBox(xStart,yStart,cutoffWidth,height);
        u8g2.drawBox(xStart+cutoffWidth,yStart,int((width-cutoffWidth)*(float(current - cutoff)/float(max - cutoff))),height);
    } 
    for (int i = 0; i<=max; i+=1000) {
        int offset;
        if (i<=cutoff) {
            offset=int((float(i)/float(max))*width)/2;
        } else {
            offset=cutoffWidth+int((float(i-cutoff)/float(max-cutoff))*(width-cutoffWidth));
        }
        if (i==redLine) {
            u8g2.drawVLine(xStart + offset, yStart, 5+height);
        } else {
            u8g2.drawVLine(xStart + offset, yStart+height, 5);
        }
        if (i%2000!=0 && i<=cutoff) {
            continue;
        }
        u8g2.setFont(u8g2_font_bitcasual_tn);
        int xfontoff = (i<10000) ? 2 : 5;
        u8g2.drawStr(xStart + offset - xfontoff, yStart+height+5+8,String(i/1000).c_str());
    }
    u8g2.setFont(u8g2_font_logisoso18_tf);
    int newFontx = 11;
    int yOff = 2;
    int xOff = 2;
    if (current<redLine) {
        //clearBox(screenx/2+25,screeny/2+29-32,screenx/2,33);
        u8g2.drawStr(xStart+width+xOff,yStart+height-yOff,"0");
        u8g2.drawStr(xStart+width+newFontx+xOff,yStart+height-yOff,String(current/1000).c_str());
    } else {
        u8g2.drawStr(xStart+width+xOff,yStart+height-yOff,String(current/1000).c_str());
        if(shiftText){
        u8g2.setFont(u8g2_font_logisoso32_tf);
        u8g2.drawStr(screenx/2+25,screeny/2+29,"SHIFT");
        }
        u8g2.setFont(u8g2_font_logisoso18_tf);
    }
    u8g2.drawStr(xStart+width+2*newFontx+xOff,yStart+height-yOff,".");
    u8g2.drawStr(xStart+width+3*newFontx+xOff-6,yStart+height-yOff,String((current%1000)/100).c_str());
    //u8g2.sendBuffer();
 }
};

class statusMessage
{
    public:
    int x0 = 0; // the starting cordinates for the status message object
    int y0 = 0;
    uint8_t* messageFont = u8g2_font_5x7_tr;
    uint8_t* statusFont = messageFont;
    //enum displayItem {date,GPS_Status,loggingStatus}; // maybe best to make these functions?
    
    private:
    int x1; // stores the end location of the message: makes screen formating easier!
    int y00; // stores the top extent of the message
    int y1 = 0; // stores the bottom extent of the message
    int messageHeight = 0;
    int messageWidth = 0;
    int statusWidth = 0;
    int statusHeight = 0;
    int status_x0 = 0;
    int status_y0 = 0;
    int statusTop = 0;

    public:
    // void setFont() // the set font function will make it so when the user changes the message font the status font will also follow.
    // {
    //     if(Nargin)
    // } 
    
    int xEnd()
    {
        int xEnd = status_x0 + statusWidth;
        return(xEnd);
    }
    void offsetStatus(int status_xOffset,int status_yOffset)
    {
        status_x0 += status_xOffset;
        status_y0 += status_yOffset;
    }

    void initialize(char * message, char * status) // performs all formatting calculations call this once
    {
        // draw the message
        u8g2.setFont(messageFont);
        int8_t messageAscent = u8g2.getAscent();
        int8_t messageDecent = u8g2.getDescent();
        messageWidth = u8g2.getStrWidth(message);
        messageHeight = messageAscent - messageDecent; // the area the message can occupy
        y1 = y0 + messageDecent; // find the absolute bottom of the message
        y00 = y0 - messageAscent; // the upper corner of the message
        u8g2.drawStr(x0,y0,message);// draw the message
        
        // draw the initial status
        u8g2.setFont(statusFont); //switch to the status font
        int8_t statusDecent = u8g2.getDescent();
        int8_t statusAscent = u8g2.getAscent();
        statusWidth = u8g2.getStrWidth(status);
        statusHeight = statusAscent - statusDecent; // the area the message can occupy
        statusTop = y0 - statusAscent; // the upper corner of the message
        status_x0 = x0 + messageWidth; // start the status text at the end of the message
        status_y0 = y0;
        u8g2.drawStr(status_x0,status_y0,status);
    }
    void display(char * status)
    {
        clearBox(status_x0,statusTop,statusWidth,statusHeight);
        u8g2.setFont(statusFont);
        u8g2.drawStr(status_x0,status_y0,status);
    }

};

class digitalGauge
{
    //todo 
    // consider adding extra fancy options like printing the units vertically to save space or having an indentifer precede the digits
    // Look at removing bloat by eliminating redundant variables instead using functions to modify key variables used in the printing process
    public:
    char printFormat [10] = "%i";
    char unitText [10] = "mph";
    int x0 = 0;
    int y0 = screeny/2+20;
    float maxVal = 199;

    uint8_t* digitFont = u8g2_font_logisoso30_tr;
    uint8_t* unitFont = u8g2_font_VCR_OSD_mf;
    
    private:
    float lastVal = 0;
    int maxDigitWidth = 0;
    int x0unit = 0;
    int y0unit = 0;
    int xUnitOffset = 0;
    int yUnitOffset = 0;
    int digitHeight = 0;
    int digit_y0 = 0;
    int digit_y1 = 0;
    uint8_t tx = 0; // used for the clear box function this is the x cordinate of the area that changes
    uint8_t ty = 0;
    uint8_t tw = 0;
    uint8_t th = 0;
    
    public:
    int xEnd()
    {
        u8g2.setFont(unitFont); // set the font to the unit font so the appropriate width can be calculated
        int totalWidth = maxDigitWidth + u8g2.getStrWidth(unitText) + xUnitOffset;
        int x1 = x0 + totalWidth; 
        // or the max of the x value of xunit+strwidth or x0+digitwidth
        return(x1);
    }
    int yEndBottom()
    {
        u8g2.setFont(digitFont); // set font to digit font to get values
        int y1 = y0 + u8g2.getDescent(); // determine how much the digit hangs down below 
        u8g2.setFont(unitFont); // set the font to the unit font
        y1 = max(y1,y0unit+u8g2.getDescent()); // the bottom of the object is just max o
        return(y1);
    }
    void unitLocation (int xOffset, int yOffset = 0)
    {
        xUnitOffset = xOffset;
        yUnitOffset = yOffset;
    }
    void findActiveArea()
    {
        //(NOTE x position of update area cannot be changed with 6963 controller with current library, but this is a possible limitation of the controller itself)
        tx = x0 / 8; // find the smallest possible value for the beginning of the title block 
        tw = ceil(float(x0 + maxDigitWidth) / 8.0) - tx; // round up to find the minimum size box that needs to be updated 
        ty = 16 - ((digit_y0) / 8); // Because display is rotated the top of the screen is actually the bottom.
        th = ceil(float(digitHeight)/8.0);
    }
    void updateRowArea()
    {
        u8g2.updateDisplayArea(0,ty,30,th); // only update the area required tile cordinates are 8 pixel blocks each
    }
    void display(float val)
    {
        if(lastVal == val)
        {
            return; // don't do anything if no updates need to be made to save time
        }
        lastVal = val; // otherwise save the latest value;
        clearBox(x0,digit_y0,maxDigitWidth,digitHeight);
        u8g2.setFont(digitFont);
        char digitString [10] = "Error"; // make it error so its ovbious if theres a problem
        sprintf(digitString,printFormat,val); // generate a string with the digits in it
        #define currentWidth u8g2.getStrWidth(digitString) // use the current width to set the location of the digits
        #define digit_x0 x0+maxDigitWidth-currentWidth
        u8g2.drawStr(digit_x0,y0,digitString);
    }

    void initialize(float val)
    {
        u8g2.setFont(digitFont);
        char digitString [10] = "Error"; // make it error so its ovbious if theres a problem
        sprintf(digitString,printFormat,maxVal); // generate a string with the digits in it
        maxDigitWidth = u8g2.getStrWidth(digitString);
        digitHeight = u8g2.getAscent() - u8g2.getDescent(); // the area the digit can occupy
        digit_y0 = y0 - u8g2.getAscent(); // the upper corner of the digit
        u8g2.setFont(unitFont);
        x0unit = x0 + maxDigitWidth + xUnitOffset;
        y0unit = y0 - yUnitOffset; 
        findActiveArea();
        u8g2.drawStr(x0unit,y0unit,unitText); // draw the unit text
        //u8g2.drawFrame(tx*8,ty*8,tw*8,th*8);
        display(val);
    }
};

boxGauge tachometer;
digitalGauge speed;
digitalGauge xAcel;
digitalGauge yAcel;
digitalGauge lat;
digitalGauge lon;
statusMessage GPS_status;
statusMessage loggingStatus;




void initializeEaganM3_Screen(int myRPM = 0)
{
    u8g2.clearBuffer();
    GPS_status.y0 = 8;
    GPS_status.initialize("GPS: ","Disconnected");
    loggingStatus.y0 = 8;
    loggingStatus.x0 = GPS_status.xEnd() + 8;
    loggingStatus.initialize("logging: ","OFF");
    //boxGauge tachometer;
    tachometer.yStart = 12;
    tachometer.redLine = 7500;
    tachometer.max = 8500;
    tachometer.cutoff = 2000;
    //u8g2.print("gps not functional");
    tachometer.drawBoxGauge(myRPM);
    //digitalGauge speed;
    strcpy(speed.printFormat,"%3i\0");
    speed.initialize(map(analogRead(A14),0,1023,0,156));
    //digitalGauge xAcel;
    xAcel.maxVal = -99;
    xAcel.x0 = speed.xEnd();
    //xAcel.y0 = 120; 
    strcpy(xAcel.unitText,"xgs\0");
    strcpy(xAcel.printFormat,"%+2i\0");
    xAcel.unitFont = u8g2_font_t0_12_tf;
    xAcel.unitLocation(1,10);
    xAcel.initialize(99);
    //digitalGauge yAcel;
    yAcel.maxVal = -99;
    strcpy(yAcel.printFormat,"%+2i\0");
    yAcel.x0 = xAcel.xEnd();
    strcpy(yAcel.unitText,"ygs\0");
    yAcel.unitFont = u8g2_font_t0_12_tf;
    yAcel.unitLocation(1,10);
    yAcel.initialize(99);
    lat.y0 = speed.yEndBottom() + 15;
    lat.digitFont = u8g2_font_6x12_mn;
    lat.unitFont = u8g2_font_5x7_tr;
    strcpy(lat.unitText,"*\0");
    strcpy(lat.printFormat,"%6.4f\0");
    lat.maxVal = -100.1234;
    lat.initialize(-100.1234);
    lon.y0 = speed.yEndBottom() + 15;
    lon.digitFont = u8g2_font_6x12_mn;
    lon.unitFont = u8g2_font_5x7_tr;
    lon.x0 = lat.xEnd() + 30;
    strcpy(lon.unitText,"*\0");
    strcpy(lon.printFormat,"%6.4f\0");
    lon.maxVal = -100.1234;
    lon.initialize(-100.1234);
    u8g2.sendBuffer();
}
void EaganM3_Screen(int myRPM = 0)
{
    if(GPSconnected)
    GPS_status.display("Connected!");
    else
    GPS_status.display("Disconnected");
    tachometer.display(map(analogRead(A14),0,1023,0,8500));
    speed.display(int(gpsSpeed));
    xAcel.display(xAccel);
    yAcel.display(yAccel);
    lat.display(latitude);
    lon.display(longitude);
    
    //speed.updateArea();
    u8g2.sendBuffer();
}
//void initilizeBoxGauge
// void displayGPSbootup()
// {
//     u8g2.setCursor(32,40);
//     u8g2.print("gps not functional");

// }