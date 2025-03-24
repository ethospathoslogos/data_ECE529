#include "address_map_arm.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>  // For sprintf

#define KEY_BASE              0xFF200050
#define VIDEO_IN_BASE         0xFF203060
#define FPGA_ONCHIP_BASE      0xC8000000

int main(void)
{
    volatile int *KEY_ptr = (int *)KEY_BASE;
    volatile int *Video_In_DMA_ptr = (int *)VIDEO_IN_BASE;
    volatile short *Video_Mem_ptr = (short *)FPGA_ONCHIP_BASE;
    
    int x = 10, y = 10;
    int counter = 0;       // Counter for the number of pictures taken
    int bw_mode = 0;       // 0 = color mode, 1 = black & white mode

    // Enable video capture initially
    *(Video_In_DMA_ptr + 3) = 0x4;
    
    // Display a timestamp once on the character buffer at (x=10, y=10)
    char timeStr[30];
    unsigned int offset = (y << 7) + x;  // Assuming character buffer layout remains the same
    time_t timer;
    struct tm *tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
    
    char *p = timeStr;
    while (*p)
    {
        *((volatile char *)(0xC9000000 + offset)) = *p;
        p++;
        offset++;
    }
    
    // Main loop: handle key events and picture capture
    while (1)
    {
        int keys = *KEY_ptr;  // Read key status (each bit corresponds to a different key)
        if (keys != 0)
        {
            // Toggle black & white mode when KEY2 is pressed (bit mask 0x4)
            if (keys & 0x4)
            {
                *(Video_In_DMA_ptr + 3) = 0x0;  // Disable video capture to freeze the frame
                while (*KEY_ptr & 0x4)          // Wait until KEY2 is released
                {
                    /* Do nothing */
                }
                // Toggle black & white mode
                if (bw_mode == 0)
                    bw_mode = 1;
                else
                    bw_mode = 0;
                *(Video_In_DMA_ptr + 3) = 0x4;  // Re-enable video capture
            }
            // For any other key press, capture a picture
            else
            {
                *(Video_In_DMA_ptr + 3) = 0x0;  // Disable video to capture (freeze) the frame
                while (*KEY_ptr != 0)           // Wait for keys to be released
                {
                    /* Do nothing */
                }
                
                counter++;  // Increment picture counter
                
                // If black & white mode is active, convert the captured image to grayscale.
                if (bw_mode)
                {
                    int row = 0;
                    while (row < 480)
                    {
                        int col = 0;
                        while (col < 640)
                        {
                            int index = row * 640 + col;
                            short pixel = *(Video_Mem_ptr + index);
                            
                            // Extract RGB565 components:
                            // Bits 15-11: Red, bits 10-5: Green, bits 4-0: Blue.
                            int red   = (pixel >> 11) & 0x1F;
                            int green = (pixel >> 5)  & 0x3F;
                            int blue  = pixel & 0x1F;
                            
                            // Convert to 8-bit per channel values:
                            int r8 = (red   * 255) / 31;
                            int g8 = (green * 255) / 63;
                            int b8 = (blue  * 255) / 31;
                            
                            // Compute grayscale value using weighted average:
                            // Using approximate integer weights (77, 150, 29) with division by 256.
                            int gray8 = (77 * r8 + 150 * g8 + 29 * b8) / 256;
                            
                            // Convert the 8-bit gray value back to RGB565 components:
                            int new_red   = (gray8 * 31) / 255;
                            int new_green = (gray8 * 63) / 255;
                            int new_blue  = (gray8 * 31) / 255;
                            
                            short new_pixel = (new_red << 11) | (new_green << 5) | new_blue;
                            *(Video_Mem_ptr + index) = new_pixel;
                            
                            col++;
                        }
                        row++;
                    }
                }
                
                // Display the updated picture counter on the character buffer at (row 20, col 10)
                char counterStr[20];
                sprintf(counterStr, "Pics: %d", counter);
                unsigned int offset_counter = (20 << 7) + 10;
                char *cp = counterStr;
                while (*cp)
                {
                    *((volatile char *)(0xC9000000 + offset_counter)) = *cp;
                    cp++;
                    offset_counter++;
                }
                
                *(Video_In_DMA_ptr + 3) = 0x4;  // Re-enable video capture for the next frame
            }
        }
    }
    
    return 0;
}
