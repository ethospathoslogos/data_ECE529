#include "address_map_arm.h"
#include <stdlib.h>

#define KEY_BASE       0xFF200050
#define VIDEO_IN_BASE  0xFF203060

int main(void)
{
    volatile int *KEY_ptr = (int *)KEY_BASE;
    volatile int *Video_In_DMA_ptr = (int *)VIDEO_IN_BASE;
    
    // Assume video is initially disabled.
    *(Video_In_DMA_ptr + 3) = 0x0;
    
    while (1)
    {
        int keys = *KEY_ptr;  // Read the key status
        
        if (keys != 0)
        {
            // If KEY3 is pressed, enable the video capture.
            if (keys & 0x8)
            {
                // Wait until KEY3 is released.
                while (*KEY_ptr & 0x8);
                // Enable video capture.
                *(Video_In_DMA_ptr + 3) = 0x4;
            }
            
            // If KEY2 is pressed, disable the video capture.
            if (keys & 0x4)
            {
                // Wait until KEY2 is released.
                while (*KEY_ptr & 0x4);
                // Disable video capture.
                *(Video_In_DMA_ptr + 3) = 0x0;
            }
        }
    }
    
    return 0;
}
