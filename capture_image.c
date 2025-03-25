#include <time.h>
#include <stdio.h>  // For sprintf
#include <stdlib.h>

#define KEY_BASE              0xFF200050
#define VIDEO_IN_BASE         0xFF203060
#define FPGA_ONCHIP_BASE      0xC8000000

int main(void)
{
    volatile int *KEY_ptr           = (int *) KEY_BASE;
    volatile int *Video_In_DMA_ptr  = (int *) VIDEO_IN_BASE;
    volatile short *Video_Mem_ptr   = (short *) FPGA_ONCHIP_BASE;
    
    int x, y;
    putenv("TZ=EST5EDT");
    tzset();
    
    x = 10;
    y = 10;
    int counter = 1; // Counter to keep track of the number of pictures taken
    
    // Enable video capture initially
    *(Video_In_DMA_ptr + 3) = 0x4;
    
    // Display timestamp once on the character buffer at (x=10, y=10)
    char timeStr[30];
    unsigned int offset = (y << 7) + x;  // Calculate character buffer address
    time_t timer;
    struct tm *tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // Print the timestamp (initialize pointer each time)
    {
        char *p = timeStr;
        unsigned int off = offset; 
        while (*p) {
            *((volatile char *)(0xC9000000 + off)) = *p;
            p++;
            off++;
        }
    }
    
    // Prepare and display the counter string at (x=10, y=20)
    char counterStr[20];
    sprintf(counterStr, "Pics: %d", counter);
    unsigned int offset_counter = (20 << 7) + 10;  // For example, row 20, column 10
    {
        char *cp = counterStr;
        unsigned int offc = offset_counter;
        while (*cp) {
            *((volatile char *)(0xC9000000 + offc)) = *cp;
            cp++;
            offc++;
        }
    }
    
    while (1)
    {
        // When KEY3 is pressed, disable video and update the timestamp.
        if ((*KEY_ptr) & 0x8)  // KEY3 pressed
        {
            *(Video_In_DMA_ptr + 3) = 0x0;  // Disable video capture
            
            // Wait until KEY3 is released
            while ((*KEY_ptr) & 0x8);
            
            // Recompute offset for timestamp display at (x=10, y=10)
            offset = (y << 7) + x;
            time(&timer);
            tm_info = localtime(&timer);
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
            
            // Reinitialize pointer and offset then display the updated timestamp.
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
        
        // When KEY2 is pressed, enable video and update the picture counter.
        if ((*KEY_ptr) & 0x4)  // KEY2 pressed
        {
            *(Video_In_DMA_ptr + 3) = 0x4;  // Enable video capture
            
            // Wait until KEY2 is released
            while ((*KEY_ptr) & 0x4);
            
            counter++;  // Increment picture counter
            
            // Prepare the new counter string.
            sprintf(counterStr, "Pics: %d", counter);
            // Recompute offset for counter display at (x=10, y=20)
            offset_counter = (20 << 7) + 10;
            
            // Reinitialize the pointer and display the new counter.
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
    
    // (The following loop is never reached.)
    for (y = 0; y < 240; y++) {
        for (x = 0; x < 320; x++) {
            short temp2 = *(Video_Mem_ptr + (y << 9) + x);
            *(Video_Mem_ptr + (y << 9) + x) = temp2;
        }
    }
    
    return 0;
}
