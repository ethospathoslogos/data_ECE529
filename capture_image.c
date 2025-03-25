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
    
    // Coordinates for display areas:
    int x_timestamp = 10, y_timestamp = 10;  // Timestamp display area
    int x_counter   = 10, y_counter   = 20;  // Counter (Pics) display area
    
    int counter = 0;  // 0: display timestamp; 1: flip vertically; 2: mirror horizontally.
    
    // Initially enable video capture.
    *(Video_In_DMA_ptr + 3) = 0x4;
    
    // Display initial timestamp (since counter is 0).
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
    
    // Display the initial picture counter ("Pics: 0") at (10,20)
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
        // --- KEY3 branch: apply effect (timestamp, flip, or mirror) ---
        if ((*KEY_ptr) & 0x8)  // KEY3 pressed
        {
            // Disable video capture (freeze the frame)
            *(Video_In_DMA_ptr + 3) = 0x0;
            
            // Wait until KEY3 is released.
            while ((*KEY_ptr) & 0x8);
            
            // Check counter to determine the effect.
            if (counter == 0)
            {
                // If counter is 0, update and display the timestamp.
                offset_timestamp = (y_timestamp << 7) + x_timestamp;
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
            else if (counter == 1)
            {
                // Flip the image upside down (vertical flip)
                int row = 0;
                while (row < 240)  // Process top half rows
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
                // Mirror the image horizontally (flip left and right)
                int row = 0;
                while (row < 480)
                {
                    int col = 0;
                    while (col < 320)  // Process half the columns
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
        
        // --- KEY2 branch: enable video and update counter ---
        if ((*KEY_ptr) & 0x4)  // KEY2 pressed
        {
            *(Video_In_DMA_ptr + 3) = 0x4;  // Enable video capture
            while ((*KEY_ptr) & 0x4);       // Wait until KEY2 is released
            
            // Increment counter and cycle it: 0->1->2->0...
            counter++;
            if (counter > 2)
            {
                counter = 0;
            }
            
            // Update the "Pics" counter display.
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
