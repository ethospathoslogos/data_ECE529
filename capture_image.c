#include <time.h>
#include <stdio.h>  // For sprintf

#define KEY_BASE              0xFF200050
#define VIDEO_IN_BASE         0xFF203060
#define FPGA_ONCHIP_BASE      0xC8000000

/* This program demonstrates the use of the D5M camera with the DE1-SoC Board
 * It performs the following: 
 * 	1. Capture one frame of video when any key is pressed.
 * 	2. Display the captured frame when any key is pressed.		  
*/
/* Note: Set the switches SW1 and SW2 to high and rest of the switches to low for correct exposure timing while compiling and the loading the program in the Altera Monitor program.
*/
int main(void)
{
	volatile int * KEY_ptr				= (int *) KEY_BASE;
	volatile int * Video_In_DMA_ptr	= (int *) VIDEO_IN_BASE;
	volatile short * Video_Mem_ptr	= (short *) FPGA_ONCHIP_BASE;

	int x, y;
    putenv("TZ=EST5EDT");

    	tzset(); 
        
        x = 10;
         y = 10;
        int counter = 1; // Counter to keep track of the number of pictures taken
    
        // Enable video capture initially
        *(Video_In_DMA_ptr + 3) = 0x4;
        int keys = *KEY_ptr;
        // Display timestamp once on the character buffer at (x=10, y=10)
        char timeStr[30];
        char *p = timeStr;
        unsigned int offset = (y << 7) + x;  // Character buffer address calculation
        time_t timer;
        struct tm* tm_info;
        time(&timer);
        tm_info = localtime(&timer);
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
        /**/
           // Prepare the counter string and display it on the character buffer at (x=10, y=20)
           char counterStr[20];
           sprintf(counterStr, "Pics: %d", counter);
           unsigned int offset_counter = (20 << 7) + 10;  // For example, row 20, column 10
           char *cp = counterStr;
   
	while (1)
	{
		if (*KEY_ptr != 0 && *KEY_ptr & 0x8)						// check if any KEY was pressed
		{
             
         
                *(Video_In_DMA_ptr + 3) = 0x0;	
                	// Disable the video to capture one frame
                while (*p) {
                    *((volatile char *)(0xC9000000 + offset)) = *p;
                    p++;
                    offset++;

                }
                
		}
        if(*KEY_ptr != 0 && *KEY_ptr & 0x16)						// check if any KEY was pressed
                    {
                        counter++;  // Increment picture counter

                        *(Video_In_DMA_ptr + 3) = 0x4;	
                            // Disable the video to capture one frame
                            while (*cp) {
                                    *((volatile char *)(0xC9000000 + offset_counter)) = *cp;
                                    cp++;
                                    offset_counter++;
                            }
                                
                        
                    
                }
        

	}
    

    


	for (y = 0; y < 240; y++) {
		for (x = 0; x < 320; x++) {
			short temp2 = *(Video_Mem_ptr + (y << 9) + x);
			*(Video_Mem_ptr + (y << 9) + x) = temp2;
		}
	}

}
