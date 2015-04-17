#include ".sspscreen.h"

int	main (void) {
	
	unsigned long	j;
	unsigned long	k;
	unsigned long	col;
	unsigned long	row;
	unsigned int	IdleCount = 0;
	int				TempColor[11] = {WHITE, BLACK, RED, GREEN, BLUE, CYAN, MAGENTA, YELLOW, BROWN, ORANGE, PINK};
	char			*TempChar[11] = {"White", "Black", "Red", "Green", "Blue", "Cyan", "Magenta", "Yellow", "Brown", "Orange", "Pink"};		

	
	// clear the screen
	LCDClearScreen();



	// draw a string
	LCDPutStr("Hello World", 60, 10, SMALL, WHITE, BLACK);
	LCDPutStr("Hello World", 40, 10, MEDIUM, ORANGE, BLACK);
	LCDPutStr("Hello World", 20, 10, LARGE, PINK, BLACK);

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
		Delay(2000000);
	}
	
	// ***************************************************************
	// *  character and line tests - draw lines, strings, etc.       *
	// ***************************************************************
	
	// clear the screen
	LCDClearScreen();
	
	// set a few pixels
	LCDSetPixel(30, 120, RED);
	LCDSetPixel(34, 120, GREEN);
	LCDSetPixel(38, 120, BLUE);
	LCDSetPixel(42, 120, WHITE);
	
	// draw some characters
	LCDPutChar('E', 10, 10, SMALL, WHITE, BLACK);
	
	// draw a string
	LCDPutStr("Hello World", 60, 10, SMALL, WHITE, BLACK);
	LCDPutStr("Hello World", 40, 10, MEDIUM, ORANGE, BLACK);
	LCDPutStr("Hello World", 20, 10, LARGE, PINK, BLACK);
	
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
	Delay(2000000);

	
	// ***************************************************************
	// *  bmp display test - display the Olimex photograph           *
	// ***************************************************************
	
	LCDClearScreen();
	
	LCDWrite130x130bmp();
	
	LCDPutStr("This guy is nuts", 115, 10, LARGE, BLACK, CYAN);

	// draw a filled box
	LCDSetRect(90, 70, 75, 120, FILL, YELLOW);
	
	LCDPutStr("HELP!", 80, 80, SMALL, BLACK, YELLOW);
}