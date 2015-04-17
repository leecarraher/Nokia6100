#ifndef SSPSCREEN_H_
#define SSPSCREEN_H_
// Booleans
#define NOFILL              0
#define FILL                1
// 12-bit color definitions
#define WHITE          0xFFF
#define BLACK          0x000
#define RED            0xF00
#define GREEN          0x0F0
#define BLUE           0x00F
#define CYAN           0x0FF
#define MAGENTA        0xF0F
#define YELLOW         0xFF0
#define BROWN          0xB22
#define ORANGE         0xFA0
#define PINK           0xF6A
// Font sizes
#define SMALL          0
#define MEDIUM         1
#define LARGE          2

static void LCDWrite130x130bmp(void);
static void LCDClearScreen(void);
static void LCDSetPixel(int color, int x, int y);
static void LCDSetLine(int x1, int y1, int x2, int y2, int color);
static void LCDSetRect(int x0, int y0, int x1, int y1, unsigned char fill, int color);
static void LCDSetCircle(int x0, int y0, int radius, int color);
static void LCDPutChar(char c, int x, int y, int size, int fcolor, int bcolor);
static void LCDPutStr(char *pString, int x, int y, int Size, int fColor, int bColor);
static void screen_delay(int n);

#endif /*SSPSCREEN_H_*/
