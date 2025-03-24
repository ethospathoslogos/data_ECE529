#include "address_map_arm.h"
#include <time.h>

#define KEY_BASE              0xFF200050
#define VIDEO_IN_BASE         0xFF203060
#define FPGA_ONCHIP_BASE      0xC8000000

/* This program demonstrates the use of the D5M camera with the DE1-SoC Board
 * It performs the following: 
 * 	1. Timestamp once for video 
 *  2.unter to keep track of number of picture 
 * 			  
*/
/* Note: Set the switches SW1 and SW2 to high and rest of the switches to low for correct exposure timing while compiling and the loading the program in the Altera Monitor program.
*/

int main(void)
{
	volatile int * KEY_ptr				= (int *) KEY_BASE;
	volatile int * Video_In_DMA_ptr	= (int *) VIDEO_IN_BASE;
	volatile short * Video_Mem_ptr	= (short *) FPGA_ONCHIP_BASE;

	int x = 10;
	int y = 10;

	*(Video_In_DMA_ptr + 3)	= 0x4;				// Enable the video
	char* buffer = (char *)malloc(20);
	unsigned int offset = (y << 7) + x;
	time_t timer;
	struct tm* tm_info;
    int counter = 0;
	time(&timer);
	tm_info = localtime(&timer);

	strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

	while(1) {

		while ( *(buffer) ) {
			*((volatile char *)(0xC9000000 + offset)) = *(buffer); // write to the character buffer
			++buffer;
			++offset;
			}
	



	}
	
	
	while (1)
	{
		

		// int alt_up_char_buffer_draw(alt_up_char_buffer_dev*char_buffer, 'a', 10,10);
		if (*KEY_ptr != 0)						// check if any KEY was pressed
		{
			*(Video_In_DMA_ptr + 3) = 0x0;			// Disable the video to capture one frame
			while (*KEY_ptr != 0);				// wait for pushbutton KEY release
			break;
		}
		
		//int alt_up_char_buffer_draw(alt_up_char_buffer_dev*char_buffer, unsigned char ch, unsigned int x,unsigned int y)
			
		//int alt_up_char_buffer_string(alt_up_char_buffer_dev*char_buffer, const char *ptr, unsigned int x,unsigned int y)
			
		// if (*KEY_ptr != 0)						// check if any KEY was pressed
		// {
		// 	*(Video_In_DMA_ptr + 3) = 0x4;			// Disable the video to capture one frame
		// 	while (*KEY_ptr != 0);				// wait for pushbutton KEY release
		// 	break;
		// }
	}

	while (1)
	{
		if (*KEY_ptr != 0)						// check if any KEY was pressed
		{
			break;
		}
	}




	for (y = 0; y < 240; y++) {
		for (x = 0; x < 320; x++) {
			short temp2 = *(Video_Mem_ptr + (y << 9) + x);
			*(Video_Mem_ptr + (y << 9) + x) = temp2;
		}
	}

}