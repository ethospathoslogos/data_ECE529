#include <time.h>
#include <stdio.h>   // For sprintf
#include <stdlib.h>

#define KEY_BASE              0xFF200050
#define VIDEO_IN_BASE         0xFF203060
#define FPGA_ONCHIP_BASE      0xC8000000

int main(void)
{
    volatile int *KEY_ptr           = (int *)KEY_BASE;
    volatile int *Video_In_DMA_ptr  = (int *)VIDEO_IN_BASE;
    volatile short *Video_Mem_ptr   = (short *)FPGA_ONCHIP_BASE;
    
    int x, y;
    putenv("TZ=EST5EDT");
    tzset();
    
    x = 10;
    y = 10;
    int counter = 0;  // Start with counter 0 so the timestamp shows
    
    // Enable video capture initially
    *(Video_In_DMA_ptr + 3) = 0x4;
    
    // Display the timestamp if counter is 0 (at row 10, col 10)
    char timeStr[30];
    unsigned int offset = (y << 7) + x;  // Character buffer address for timestamp
    time_t timer;
    struct tm *tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
    
    
    // Prepare and display the counter string (at row 20, col 10)
    char counterStr[20];
    sprintf(counterStr, "Pics: %d", counter);
    unsigned int offset_counter = (20 << 7) + 10;
    {
        char *cp = counterStr;
        unsigned int offc = offset_counter;
        while(*cp) {
            *((volatile char *)(0xC9000000 + offc)) = *cp;
            cp++;
            offc++;
        }
    }
    
    while (1)
    {
        // If KEY3 is pressed, disable video capture.
        if ((*KEY_ptr) & 0x8)
        {
            *(Video_In_DMA_ptr + 3) = 0x0;  // Disable video capture
            // Wait until KEY3 is released.
            while ((*KEY_ptr) & 0x8);
            
            // Recompute offset for timestamp display area (row 10, col 10)
            offset = (y << 7) + x;
            if (counter == 0)
            {
                // If counter is 0, update and display the timestamp.
                time(&timer);
                tm_info = localtime(&timer);
                strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
                {
                    char *p = timeStr;
                    unsigned int off = offset;
                    while (*p) {
                        *((volatile char *)(0xC9000000 + off)) = *p;
                        p++;
                        off++;
                    }
                }
            }
            else
            {
                // Otherwise, blank out the timestamp area (write 30 spaces).
                int i = 0;
                unsigned int off = offset;
                while(i < 30) {
                    *((volatile char *)(0xC9000000 + off)) = ' ';
                    i++;
                    off++;
                }
            }
        }
        
        // If KEY2 is pressed, enable video capture and update the counter.
        if ((*KEY_ptr) & 0x4)
        {
            *(Video_In_DMA_ptr + 3) = 0x4;  // Enable video capture
            // Wait until KEY2 is released.
            while ((*KEY_ptr) & 0x4);
            
            counter++;  // Increment picture counter
            
            // Update the counter display.
            sprintf(counterStr, "Pics: %d", counter);
            offset_counter = (20 << 7) + 10;  // Reset display offset for counter
            {
                char *cp = counterStr;
                unsigned int offc = offset_counter;
                while (*cp) {
                    *((volatile char *)(0xC9000000 + offc)) = *cp;
                    cp++;
                    offc++;
                }
            }
        }
    }
    
    // (This part of the code is never reached.)
    return 0;
}
