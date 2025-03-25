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
    
    // Display coordinates for the character buffer:
    int x_timestamp = 10, y_timestamp = 10;  // Timestamp display area
    int x_counter   = 10, y_counter   = 20;   // "Pics" counter display area
    
    // Mode counter:
    // 0 => update & display timestamp,
    // 1 => vertical flip,
    // 2 => horizontal mirror,
    // 5 => black and white conversion.
    // The counter is incremented by KEY2 (and is never reset).
    int counter = 0;
    
    // Initially, enable video capture.
    *(Video_In_DMA_ptr + 3) = 0x4;
    
    // Initially display the timestamp if counter == 0.
    if (counter == 0)
    {
        char timeStr[30];
        unsigned int offset_timestamp = (y_timestamp << 7) + x_timestamp;
        time_t timer;
        struct tm *tm_info;
        time(&timer);
        tm_info = localtime(&timer);
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
        {
            char *p = timeStr;
            unsigned int off = offset_timestamp;
            while (*p)
            {
                *((volatile char *)(0xC9000000 + off)) = *p;
                p++;
                off++;
            }
        }
    }
    
    // Display the initial "Pics" counter at (10,20).
    char counterStr[20];
    sprintf(counterStr, "Pics: %d", counter);
    unsigned int offset_counter = (y_counter << 7) + x_counter;
    {
        char *cp = counterStr;
        unsigned int offc = offset_counter;
        while (*cp)
        {
            *((volatile char *)(0xC9000000 + offc)) = *cp;
            cp++;
            offc++;
        }
    }
    
    while (1)
    {
        // --- KEY3 branch: capture the image and apply effect (and clear timestamp if needed) ---
        if ((*KEY_ptr) & 0x8)  // KEY3 pressed
        {
            // Disable video capture to freeze the current frame.
            *(Video_In_DMA_ptr + 3) = 0x0;
            
            // Wait until KEY3 is released.
            while ((*KEY_ptr) & 0x8);
            
            // If counter is 0, update and display the timestamp.
            // Otherwise, clear the timestamp area.
            unsigned int offset_timestamp = (y_timestamp << 7) + x_timestamp;
            if (counter == 0)
            {
                char timeStr[30];
                time_t timer;
                struct tm *tm_info;
                time(&timer);
                tm_info = localtime(&timer);
                strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
                {
                    char *p = timeStr;
                    unsigned int off = offset_timestamp;
                    while (*p)
                    {
                        *((volatile char *)(0xC9000000 + off)) = *p;
                        p++;
                        off++;
                    }
                }
            }
            else
            {
                // Clear the timestamp area by writing spaces (assume 30 characters).
                int i = 0;
                unsigned int off = offset_timestamp;
                while (i < 30)
                {
                    *((volatile char *)(0xC9000000 + off)) = ' ';
                    i++;
                    off++;
                }
            }
            
            // Apply image effect based on counter.
            if (counter == 1)
            {
                // Vertical flip: swap each row in the top half with its corresponding row in the bottom half.
                int row = 0;
                while (row < 240)  // For 480 rows, process top half rows 0 to 239.
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
            else if (counter == 2)
            {
                // Horizontal mirror: swap pixels from left half with right half.
                int row = 0;
                while (row < 480)
                {
                    int col = 0;
                    while (col < 320)  // For each row, process columns 0 to 319.
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
            else if (counter == 5)
            {
                // Convert the image to black and white (grayscale).
                int row = 0;
                while (row < 480)
                {
                    int col = 0;
                    while (col < 640)
                    {
                        int index = row * 640 + col;
                        short pixel = *(Video_Mem_ptr + index);
                        
                        // Extract RGB565 components.
                        int red   = (pixel >> 11) & 0x1F;
                        int green = (pixel >> 5)  & 0x3F;
                        int blue  = pixel & 0x1F;
                        
                        // Convert to 8-bit per channel.
                        int r8 = (red * 255) / 31;
                        int g8 = (green * 255) / 63;
                        int b8 = (blue * 255) / 31;
                        
                        // Compute grayscale value using a weighted average.
                        int gray8 = (77 * r8 + 150 * g8 + 29 * b8) / 256;
                        
                        // Convert back to RGB565.
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
            // The processed image remains frozen (video remains disabled).
        }
        
        // --- KEY2 branch: re-enable video and update counter ---
        if ((*KEY_ptr) & 0x4)  // KEY2 pressed
        {
            // Re-enable video capture.
            *(Video_In_DMA_ptr + 3) = 0x4;
            
            // Wait until KEY2 is released.
            while ((*KEY_ptr) & 0x4);
            
            // Increment the counter (counter is not reset; it only increases).
            counter++;
            
            // Update the "Pics" counter display.
            char counterStr[20];
            sprintf(counterStr, "Pics: %d", counter);
            offset_counter = (y_counter << 7) + x_counter;
            {
                char *cp = counterStr;
                unsigned int offc = offset_counter;
                while (*cp)
                {
                    *((volatile char *)(0xC9000000 + offc)) = *cp;
                    cp++;
                    offc++;
                }
            }
        }
    }
    
    return 0;
}
