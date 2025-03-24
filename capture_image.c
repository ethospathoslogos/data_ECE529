#include "address_map_arm.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define KEY_BASE              0xFF200050
#define VIDEO_IN_BASE         0xFF203060
#define FPGA_ONCHIP_BASE      0xC8000000

int main(void)
{
    volatile int * KEY_ptr = (int *) KEY_BASE;
    volatile int * Video_In_DMA_ptr = (int *) VIDEO_IN_BASE;
    volatile short * Video_Mem_ptr = (short *) FPGA_ONCHIP_BASE;

    int x = 10, y = 10;
    int counter = 0;  // Counter to track the number of pictures taken

    // Enable video capture initially
    *(Video_In_DMA_ptr + 3) = 0x4;
    
    // Display timestamp at (x=10, y=10)
    char timeStr[30];
    unsigned int offset = (y << 7) + x;
    time_t timer;
    struct tm* tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
    
    char *p = timeStr;
    while(*p) {
        *((volatile char *)(0xC9000000 + offset)) = *p;
        p++;
        offset++;
    }

    // Initialize last capture time
    time_t last_capture_time;
    time(&last_capture_time);

    // Main loop: automatically capture an image every second
    while (1) {
        time_t current_time;
        time(&current_time);
        if (current_time - last_capture_time >= 1) {
            // Disable video capture to freeze the frame (simulate image capture)
            *(Video_In_DMA_ptr + 3) = 0x0;

            // Increment the counter for the picture taken
            counter++;

            // Prepare the counter string and display it at (x=10, y=20)
            char counterStr[20];
            sprintf(counterStr, "Pics: %d", counter);
            unsigned int offset_counter = ((20) << 7) + 10;
            char *cp = counterStr;
            while (*cp) {
                *((volatile char *)(0xC9000000 + offset_counter)) = *cp;
                cp++;
                offset_counter++;
            }

            // Re-enable video capture for the next image
            *(Video_In_DMA_ptr + 3) = 0x4;

            // Update the last capture time to the current time
            last_capture_time = current_time;
        }
    }
    
    return 0;
}
