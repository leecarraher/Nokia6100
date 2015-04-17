#include <asm/arch/hardware.h>
#include "sspscreen.h"
#include "sspscreen-drv.h"
#include "fonts.h"
#include "bmp.h" 
#include <unistd.h>
#include <stdio.h>


//prerequisits 
//mknod /dev/screen c 245 0
// chmod 666 /dev/screen


FILE * screen;
char dummy;
static void getScreenDevice()
{
	/* Opening the device screen */
  screen=fopen("/dev/screen","w");
	/*remove the buffer from the file i/o */
  setvbuf(screen,&dummy,_IONBF,1);
}
 
static void detachScreenDevice()
{
  fclose(screen);
}

// define a link to the memory location of the configured ssp device
//**********************************************************************
// SEND BIT/DATA/COMMAND
static void send_cmd(unsigned char b)
{
   int value;
   value = b;
	 fwrite(&value,1,1,screen);
}

static void send_data(unsigned char b)
{
   int value;
   value = b | 0x100;
   fwrite(&value,1,1,screen);
}


// Delay, this may need some tuning as i still seem to be getting noise
static void screen_delay(int n)
{
		usleep (n*1000);//delay
}


//**********************************************************************
// PUT PIXEL
static void LCDSetPixel(int color, int x, int y)
{
         // Row address set (command 0x2B)
         send_cmd(PASET);
         send_data(x);
         send_data(x);
         // Column address set (command 0x2A)
         send_cmd(CASET);
         send_data(y);
         send_data(y);
         // Now illuminate the pixel (2nd pixel will be ignored)
         send_cmd(RAMWR);
         send_data((color >> 4) & 0xFF);
         send_data(((color & 0xF) << 4) | ((color >> 8) & 0xF));
         send_data(color & 0xFF);
}

static void LCDSetLine(int x0, int y0, int x1, int y1, int color) {
        int dy = y1 - y0;
        int dx = x1 - x0;
        int stepx, stepy;
       if  (dy < 0) { dy = -dy;    stepy = -1; }  else { stepy = 1; }
       if  (dx < 0) { dx = -dx;    stepx = -1; }  else { stepx = 1; }
       dy  <<= 1;                    // dy is now 2*dy
       dx  <<= 1;                    // dx is now 2*dx
                  LCDSetPixel(color,x0, y0);
       if (dx > dy) {
            int fraction = dy - (dx >> 1); // same as 2*dy - dx
            while (x0 != x1) {
                if (fraction >= 0) {
                     y0 += stepy;
                     fraction -= dx;          // same as fraction -= 2*dx
                }
                x0 += stepx;
                fraction += dy;               // same as fraction -= 2*dy
                LCDSetPixel(color,x0, y0);
            }
       } else {
            int fraction = dx - (dy >> 1);
            while (y0 != y1) {
                if (fraction >= 0) {
                     x0 += stepx;
                     fraction -= dy;
                }
                y0 += stepy;
                fraction += dx;
                LCDSetPixel(color,x0, y0);
            }
       }
}

static void LCDSetRect(int x0, int y0, int x1, int y1, unsigned char fill, int color) {
         int      xmin, xmax, ymin, ymax;
         int               i;
         // check if the rectangle is to be filled
         if (fill == FILL) {
                  // best way to create a filled rectangle is to define a drawing box
                  // and loop two pixels at a time
                  // calculate the min and max for x and y directions
                  xmin = (x0 <= x1) ? x0 : x1;
                  xmax = (x0 > x1) ? x0 : x1;
                  ymin = (y0 <= y1) ? y0 : y1;
                  ymax = (y0 > y1) ? y0 : y1;
                  // specify the controller drawing box according to those limits
                  // Row address set (command 0x2B)
                  send_cmd(PASET);
                  send_data(xmin);
                  send_data(xmax);
                  // Column address set (command 0x2A)
                  send_cmd(CASET);
                  send_data(ymin);
                  send_data(ymax);
                  // WRITE MEMORY
                  send_cmd(RAMWR);
                  // loop on total number of pixels / 2
                  for (i = 0; i < ((((xmax - xmin + 1) * (ymax - ymin + 1)) / 2) + 130); i++) {
                           // use the color value to output three data bytes covering two pixels
                           send_data((color >> 4) & 0xFF);
                           send_data(((color & 0xF) << 4) | ((color >> 8) & 0xF));
                           send_data(color & 0xFF);
                  }
         } else {
                  // best way to draw un  unfilled rectangle is to draw four lines
                  LCDSetLine(x0, y0, x1,  y0, color);
                  LCDSetLine(x0, y1, x1,  y1, color);
                  LCDSetLine(x0, y0, x0,  y1, color);
                  LCDSetLine(x1, y0, x1,  y1, color);
    }
}

static void LCDSetCircle(int x0, int y0, int radius, int color) {
         int f = 1 - radius;
         int ddF_x = 0;
         int ddF_y = -2 * radius;
         int x = 0;
         int y = radius;
         LCDSetPixel(color,x0, y0 + radius);
         LCDSetPixel(color,x0, y0 - radius);
         LCDSetPixel(color,x0 + radius, y0);
         LCDSetPixel(color,x0 - radius, y0);
         while(x < y) {
                  if(f >= 0) {
                           y--;
                           ddF_y += 2;
                           f += ddF_y;
                  }
                  x++;
                  ddF_x += 2;
                  f += ddF_x + 1;
                  LCDSetPixel(color,x0 + x, y0     +  y);
                  LCDSetPixel(color,x0 - x, y0     +  y);
                  LCDSetPixel(color,x0 + x, y0     -  y);
                  LCDSetPixel(color,x0 - x, y0     -  y);
                  LCDSetPixel(color,x0 + y, y0     +  x);
                  LCDSetPixel(color,x0 - y, y0     +  x);
                  LCDSetPixel(color,x0 + y, y0     -  x);
                  LCDSetPixel(color,x0 - y, y0     -  x);
         }
}

static void LCDPutChar(char c, int x, int y, int size, int fColor, int bColor) {
         extern const unsigned char FONT6x8[97][8];
         extern const unsigned char FONT8x8[97][8];
         extern const unsigned char FONT8x16[97][16];
         int                                          i,j;
         unsigned int               nCols;
         unsigned int               nRows;
         unsigned int               nBytes;
         unsigned char              PixelRow;
         unsigned char              Mask;
         unsigned int               Word0;
         unsigned int               Word1;
         unsigned char              *pFont;
         unsigned char              *pChar;
         unsigned char              *FontTable[] = {(unsigned char *)FONT6x8, (unsigned char *)FONT8x8,
                                                     (unsigned char *)FONT8x16};
         // get pointer to the beginning of the selected font table
         pFont = (unsigned char *)FontTable[size];
         // get the nColumns, nRows and nBytes
         nCols = *pFont;
         nRows = *(pFont + 1);
         nBytes = *(pFont + 2);
         // get pointer to the last byte of the desired character
         pChar = pFont + (nBytes * (c - 0x1F)) + nBytes - 1;
         // Row address set (command 0x2B)
         send_cmd(PASET);
         send_data(x);
         send_data(x + nRows - 1);
         // Column address set (command 0x2A)
         send_cmd(CASET);
         send_data(y);
         send_data(y + nCols - 1);
         // WRITE MEMORY
         send_cmd(RAMWR);
         // loop on each row, working backwards from the bottom to the top
         for (i = nRows - 1; i >= 0; i--) {
                  // copy pixel row from font table and then decrement row
                  PixelRow = *pChar--;
                  // loop on each pixel in the row (left to right)
                  // Note: we do two pixels each loop
                  Mask = 0x80;
                  for (j = 0; j < nCols; j += 2) {
                           // if pixel bit set, use foreground color; else use the background color
                           // now get the pixel color for two successive pixels
                           if ((PixelRow & Mask) == 0)
                                    Word0 = bColor;
                           else
                                    Word0 = fColor;
                           Mask = Mask >> 1;
                           if ((PixelRow & Mask) == 0)
                                    Word1 = bColor;
                           else
                                    Word1 = fColor;
                           Mask = Mask >> 1;
                           // use this information to output three data bytes
                           send_data((Word0 >> 4) & 0xFF);
                           send_data(((Word0 & 0xF) << 4) | ((Word1 >> 8) & 0xF));
                           send_data(Word1 & 0xFF);
                  }
         }
         // terminate the Write Memory command
         send_cmd(NOP);
}

static void LCDPutStr(char *pString, int x, int y, int Size, int fColor, int bColor) {
         // loop until null-terminator is seen
         while (*pString != 0x00) {
                  // draw the character
                  LCDPutChar(*pString++, x, y, Size, fColor, bColor);
                  // advance the y position
                  if (Size == SMALL)
                           y = y + 6;
                  else if (Size == MEDIUM)
                           y = y + 8;
                  else
                           y = y + 8;
                  // bail out if y exceeds 131
                  if (y > 131) break;
         }
}

static void LCDClearScreen(void) {
         long     i;                        // loop counter
         // Row address set (command 0x2B)
         send_cmd(PASET);
         send_data(0);
         send_data(131);
         // Column address set (command 0x2A)
         send_cmd(CASET);
         send_data(0);
         send_data(131);
         // set the display memory to BLACK
         send_cmd(RAMWR);
         for(i = 0; i < ((131 * 131) / 2); i++) {
                           send_data((BLACK >> 4) & 0xFF);
                           send_data(((BLACK & 0xF) << 4) | ((BLACK >> 8) & 0xF));
                           send_data(BLACK & 0xFF);
         }

}

static void LCDWrite130x130bmp(void) {
         long j;// loop counter
         // Data control (need to set "normal" page address for Olimex photograph)
         send_cmd(DATCTL);
         send_data(0x00); // P1: 0x00 = page address normal, column address normal, address scan in column direction
         send_data(0x00); // P2: 0x00 = RGB sequence (default value)
         send_data(0x02); // P3: 0x02 = Grayscale -> 16
         // Display OFF
         send_cmd(DISOFF);
         // Column address set (command 0x2A)
         send_cmd(CASET);
         send_data(0);
         send_data(131);
         // Page address set (command 0x2B)
         send_cmd(PASET);
         send_data(0);
         send_data(131);
         // WRITE MEMORY
         send_cmd(RAMWR);
         for(j = 0; j < 25740; j++) {
                  send_data(bmp[j]);
         }
         // Data control (return to  "inverted" page address)
         send_cmd(DATCTL);
         send_data(0x01); // P1:  0x01 = page address inverted, column address normal, address scan in column direction
         send_data(0x00); // P2:  0x00 = RGB sequence (default value)
         send_data(0x02); // P3:  0x02 = Grayscale -> 16
         // Display On
         send_cmd(DISON);


	// wait a bit
	screen_delay(300);

         // Data control (need to set "normal" page address for Olimex photograph)
         send_cmd(DATCTL);
         send_data(0x00); // P1: 0x00 = page address normal, column address normal, address scan in column direction
         send_data(0x00); // P2: 0x00 = RGB sequence (default value)
         send_data(0x02); // P3: 0x02 = Grayscale -> 16
         // Display OFF
         send_cmd(DISOFF);
         // Column address set (command 0x2A)
         send_cmd(CASET);
         send_data(0);
         send_data(131);
         // Page address set (command 0x2B)
         send_cmd(PASET);
         send_data(0);
         send_data(131);
         // WRITE MEMORY
         send_cmd(RAMWR);
         for(j = 0; j < 25740; j++) {
                  send_data(bmp1[j]);
         }
         // Data control (return to  "inverted" page address)
         send_cmd(DATCTL);
         send_data(0x01); // P1:  0x01 = page address inverted, column address normal, address scan in column direction
         send_data(0x00); // P2:  0x00 = RGB sequence (default value)
         send_data(0x02); // P3:  0x02 = Grayscale -> 16
         // Display On
         send_cmd(DISON);
}


void testLCD(void) {
	unsigned long	j;
	int	TempColor[11] = {WHITE, BLACK, RED, GREEN, BLUE, CYAN, MAGENTA, YELLOW, BROWN, ORANGE, PINK};
	char			*TempChar[11] = {"White", "Black", "Red", "Green", "Blue", "Cyan", "Magenta", "Yellow", "Brown", "Orange", "Pink"};		

	
	// clear the screen
	LCDClearScreen();

	// draw a string
	LCDPutStr("Hello World", 60, 10, SMALL, WHITE, BLACK);
	LCDPutStr("Hello World", 40, 10, MEDIUM, ORANGE, BLACK);
	LCDPutStr("Hello World", 20, 10, LARGE, PINK, BLACK);

	screen_delay(100);
	// clear the screen
	LCDClearScreen();

	LCDSetRect(120, 60, 80, 80, FILL, BROWN);
	
	// draw a empty box
	LCDSetRect(120, 85, 80, 105, NOFILL, CYAN);

	// draw some lines
	LCDSetLine(120, 10, 120, 50, YELLOW);
	LCDSetLine(120, 50, 80, 50, YELLOW);
	LCDSetLine(80, 50, 80, 10, YELLOW);
	LCDSetLine(80, 10, 120, 10, YELLOW);
	
	LCDSetLine(120, 85, 80, 105, YELLOW);
	LCDSetLine(80, 85, 120, 105, YELLOW);
	
	// draw a circle
	LCDSetCircle(65, 100, 30, RED);
	
	screen_delay(100);
	// clear the screen
	LCDClearScreen();

	// ***************************************************************
	// *  color test - show boxes of different colors                *
	// ***************************************************************
	for (j = 0; j < 11; j++) {
		
		// draw a filled box
		LCDSetRect(120, 10, 25, 120, FILL, TempColor[j]);
		
		// label the color
		LCDPutStr("        ", 5, 40, LARGE, BLACK, BLACK);
		LCDPutStr(TempChar[j], 5, 40, LARGE, YELLOW, BLACK);
		
		// wait a bit
		screen_delay(100);
	}
	
	// ***************************************************************
	// *  character and line tests - draw lines, strings, etc.       *
	// ***************************************************************
	
	// clear the screen
	LCDClearScreen();
	
	// draw a string
	LCDPutStr("Small Font", 60, 10, SMALL, WHITE, BLACK);
	LCDPutStr("Medium Font", 40, 10, MEDIUM, BLACK, BLACK);
	LCDPutStr("Large Font", 20, 10, LARGE, BLUE, BLACK);
	
	// draw a filled box
	LCDSetRect(120, 60, 80, 80, FILL, BROWN);
	
	// draw a empty box
	LCDSetRect(120, 85, 80, 105, NOFILL, CYAN);

	// draw some lines
	LCDSetLine(120, 10, 120, 50, YELLOW);
	LCDSetLine(120, 50, 80, 50, YELLOW);
	LCDSetLine(80, 50, 80, 10, YELLOW);
	LCDSetLine(80, 10, 120, 10, YELLOW);


	LCDSetLine(120, 85, 80, 105, YELLOW);
	LCDSetLine(80, 85, 120, 105, YELLOW);
	
	// draw a circle
	LCDSetCircle(65, 100, 10, RED);
	
	// wait a bit
	screen_delay(200);

	// ***************************************************************
	// *  bmp display test - display the Olimex photograph           *
	// ***************************************************************
	
	LCDClearScreen();
	
	LCDPutStr("Pictures And Text", 115, 10, SMALL, BLACK, WHITE);

	// draw a filled box
	LCDSetRect(90, 70, 75, 120, FILL, YELLOW);
	
	LCDWrite130x130bmp();
	LCDPutStr("More Strings", 80, 15, SMALL, BLACK, YELLOW);

}
