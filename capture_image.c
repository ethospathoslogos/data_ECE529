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
    int frame_count = 0;
    int fps = 0;

    // Enable continuous video capture
    *(Video_In_DMA_ptr + 3) = 0x4;
    
    // Display a timestamp once at (10,10)
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
    
    // Initialize timer for FPS calculation
    time_t start_time;
    time(&start_time);

    // Main loop: simulate frame processing and update FPS each second
    while (1) {
        // Simulate frame capture: each loop iteration is treated as a new frame.
        // In an actual implementation, use a hardware flag or interrupt.
        frame_count++;

        // Dummy operation to simulate processing of the frame
        // (e.g., reading the frame from video memory)
        int dummy = 0;
        for (y = 0; y < 240; y++) {
            for (x = 0; x < 320; x++) {
                dummy += *(Video_Mem_ptr + (y << 9) + x);
            }
        }

        // Check if one second has elapsed
        time_t current_time;
        time(&current_time);
        if (current_time - start_time >= 1) {
            fps = frame_count;  // Number of frames processed in the last second
            frame_count = 0;    // Reset counter for the next interval
            start_time = current_time; // Reset the timer

            // Display the FPS on the character display at (column=10, row=20)
            char fpsStr[20];
            sprintf(fpsStr, "FPS: %d", fps);
            unsigned int offset_fps = (20 << 7) + 10;
            char *cp = fpsStr;
            while (*cp) {
                *((volatile char *)(0xC9000000 + offset_fps)) = *cp;
                cp++;
                offset_fps++;
            }
        }
    }
    
    return 0;
}
