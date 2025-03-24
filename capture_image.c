#include "address_map_arm.h"
#include <time.h>
#include <stdlib.h>

#define KEY_BASE              0xFF200050
#define VIDEO_IN_BASE         0xFF203060
#define FPGA_ONCHIP_BASE      0xC8000000

int main(void)
{
    volatile int * KEY_ptr = (int *) KEY_BASE;
    volatile int * Video_In_DMA_ptr = (int *) VIDEO_IN_BASE;
    volatile short * Video_Mem_ptr = (short *) FPGA_ONCHIP_BASE;

    int x = 10;
    int y = 10;
    int counter = 0;  // Counter to keep track of the number of pictures taken

    // Enable video capture initially
    *(Video_In_DMA_ptr + 3) = 0x4;
    
    // Display timestamp once on the character buffer
    char buffer[30];
    unsigned int offset = (y << 7) + x;
    time_t timer;
    struct tm* tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    
    char *p = buffer;
    while (*p) {
        *((volatile char *)(0xC9000000 + offset)) = *p;
        p++;
        offset++;
    }

    // Main loop: wait for key press to capture picture
    while (1) {
        if (*KEY_ptr != 0) {            // Check if any KEY is pressed
            *(Video_In_DMA_ptr + 3) = 0x0;  // Disable video to capture the frame
            counter++;                // Increment picture counter

            // (Optional) Display or log the current counter value here

            // Wait for the KEY to be released
            while (*KEY_ptr != 0);
            
            // Re-enable video capture for next frame
            *(Video_In_DMA_ptr + 3) = 0x4;
        }
    }
    
    return 0;
}
