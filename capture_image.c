#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define KEY_BASE              0xFF200050
#define VIDEO_IN_BASE         0xFF203060
#define FPGA_ONCHIP_BASE      0xC8000000

int main(void)
{
    volatile int *KEY_ptr = (int *)KEY_BASE;
    volatile int *Video_In_DMA_ptr = (int *)VIDEO_IN_BASE;
    volatile short *Video_Mem_ptr = (short *)FPGA_ONCHIP_BASE;
    
    int counter = 1; // Assume counter is set externally (1 or 2 for the two effects)
    
    // Initially enable video capture.
    *(Video_In_DMA_ptr + 3) = 0x4;
    
    while (1)
    {
        // Check if KEY3 is pressed (bit mask 0x8)
        if ((*KEY_ptr) & 0x8)
        {
            // Disable video capture to freeze the current frame.
            *(Video_In_DMA_ptr + 3) = 0x0;
            
            // Wait until KEY3 is released.
            while ((*KEY_ptr) & 0x8);
            
            // If counter is 1, flip the camera upside down (vertical flip)
            if (counter == 1)
            {
                // For a 640x480 image, swap the top half with the bottom half.
                int row = 0;
                while (row < 240)  // Only need to process the first half (0 to 239)
                {
                    int col = 0;
                    while (col < 640)
                    {
                        int index_top = row * 640 + col;
                        int index_bottom = (479 - row) * 640 + col;
                        short temp = *(Video_Mem_ptr + index_top);
                        *(Video_Mem_ptr + index_top) = *(Video_Mem_ptr + index_bottom);
                        *(Video_Mem_ptr + index_bottom) = temp;
                        col++;
                    }
                    row++;
                }
            }
            // If counter is 2, mirror the camera (horizontal flip)
            else if (counter == 2)
            {
                int row = 0;
                while (row < 480)
                {
                    int col = 0;
                    while (col < 320)  // Only process half the columns
                    {
                        int index_left = row * 640 + col;
                        int index_right = row * 640 + (639 - col);
                        short temp = *(Video_Mem_ptr + index_left);
                        *(Video_Mem_ptr + index_left) = *(Video_Mem_ptr + index_right);
                        *(Video_Mem_ptr + index_right) = temp;
                        col++;
                    }
                    row++;
                }
            }
            
            // Re-enable video capture after applying the effect.
            *(Video_In_DMA_ptr + 3) = 0x4;
        }
        // (Other key-handling code could be added here if desired.)
    }
    
    return 0;
}
