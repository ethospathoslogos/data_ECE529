


// #include "address_map_arm.h"
// #include <time.h>
// #include <stdlib.h>
// #include <stdio.h>  // For sprintf

// #define KEY_BASE              0xFF200050
// #define VIDEO_IN_BASE         0xFF203060
// #define FPGA_ONCHIP_BASE      0xC8000000

// int main(void)
// {
//     volatile int * KEY_ptr = (int *) KEY_BASE;
//     volatile int * Video_In_DMA_ptr = (int *) VIDEO_IN_BASE;
//     volatile short * Video_Mem_ptr = (short *) FPGA_ONCHIP_BASE;

//     int x = 10;
//     int y = 10;
//     int counter = 0;  // Counter to keep track of the number of pictures taken

//     // Enable video capture initially
//     *(Video_In_DMA_ptr + 3) = 0x4;
    
//     // Display timestamp once on the character buffer at (x=10, y=10)
//     char timeStr[30];
//     unsigned int offset = (y << 7) + x;
//     time_t timer;
//     struct tm* tm_info;
//     time(&timer);
//     tm_info = localtime(&timer);
//     strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
    
//     char *p = timeStr;
//     while (*p) {
//         *((volatile char *)(0xC9000000 + offset)) = *p;
//         p++;
//         offset++;
//     }

//     // Main loop: wait for key press to capture picture and update picture counter display
//     while (1) {
//         if (*KEY_ptr != 0) {            // Check if any KEY is pressed
//             *(Video_In_DMA_ptr + 3) = 0x0;  // Disable video to capture the frame
// 			while (*KEY_ptr != 0);		// wait for pushbutton KEY release
			
//             counter++;                // Increment picture counter

//             // Prepare the counter string and display it on the character buffer
//             char counterStr[20];
//             sprintf(counterStr, "Pics: %d", counter);
//             // For example, display at row 20, column 10 (adjust as needed)
//             unsigned int offset_counter = ((20) << 7) + 10;
//             char *cp = counterStr;
//             while (*cp) {
//                 *((volatile char *)(0xC9000000 + offset_counter)) = *cp;
//                 cp++;
//                 offset_counter++;
//             }
+//             // Wait for the KEY to be released
//             while (*KEY_ptr != 0);

//             // Re-enable video capture for the next frame
//             *(Video_In_DMA_ptr + 3) = 0x4;
//         }
//     }
    
//     return 0;
// }



// #define KEY_BASE              0xFF200050
// #define VIDEO_IN_BASE         0xFF203060
// #define FPGA_ONCHIP_BASE      0xC8000000

// int main(void)
// {
//     volatile int * KEY_ptr = (int *) KEY_BASE;
//     volatile int * Video_In_DMA_ptr = (int *) VIDEO_IN_BASE;
//     volatile short * Video_Mem_ptr = (short *) FPGA_ONCHIP_BASE;
    
// 	putenv("TZ=EST5EDT");

// 	tzset(); 
	
//     int x = 10;
//     int y = 10;
//     int counter = 0;  // Counter to keep track of the number of pictures taken

//     // Enable video capture initially
//     *(Video_In_DMA_ptr + 3) = 0x4;
    
//     // Display timestamp once on the character buffer at (x=10, y=10)
//     char timeStr[30];
//     unsigned int offset = (y << 7) + x;  // Character buffer address calculation
//     time_t timer;
//     struct tm* tm_info;
//     time(&timer);
//     tm_info = localtime(&timer);
//     strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
//     /**/
//     char *p = timeStr;
//     while (*p) {
//         *((volatile char *)(0xC9000000 + offset)) = *p;
//         p++;
//         offset++;
//     }

//     // Main loop: check for key press events
//     while (1) {
//         int keys = *KEY_ptr; // Read the key status (each bit corresponds to a different key)
//         if (keys != 0) {
//             // Check if KEY3 is pressed (bit mask 0x8)
//             if (keys & 0x8) {
//                 // Disable video capture to freeze the frame
//                 *(Video_In_DMA_ptr + 3) = 0x0;
//                 // Wait until KEY3 is released
//                 while (*KEY_ptr & 0x8);

//                 // Mirror flip the image horizontally for a 640x480 image using while loops:
//                 int row = 0;
//                 while (row < 480) {
//                     int col = 0;
//                     while (col < 320) {  // 320 is half of 640
//                         int left_index = row * 640 + col;
//                         int right_index = row * 640 + (639 - col);
//                         short temp = *(Video_Mem_ptr + left_index);
//                         *(Video_Mem_ptr + left_index) = *(Video_Mem_ptr + right_index);
//                         *(Video_Mem_ptr + right_index) = temp;
//                         col++;
//                     }
//                     row++;
//                 }

//                 // Re-enable video capture after mirror flip
//                 *(Video_In_DMA_ptr + 3) = 0x4;
//             }
//             else {  // For any other key press, perform normal picture capture
//                 *(Video_In_DMA_ptr + 3) = 0x0;  // Disable video to capture the frame
//                 while (*KEY_ptr != 0);          // Wait for key release
                
//                 counter++;  // Increment picture counter

//                 // Prepare the counter string and display it on the character buffer at (x=10, y=20)
//                 char counterStr[20];
//                 sprintf(counterStr, "Pics: %d", counter);
//                 unsigned int offset_counter = (20 << 7) + 10;  // For example, row 20, column 10
//                 char *cp = counterStr;
//                 while (*cp) {
//                     *((volatile char *)(0xC9000000 + offset_counter)) = *cp;
//                     cp++;
//                     offset_counter++;
//                 }

//                 // Ensure key is released and re-enable video capture for the next frame
//                 while (*KEY_ptr != 0);
//                 *(Video_In_DMA_ptr + 3) = 0x4;
//             }
//         }
//     }
    
//     return 0;
// }

#include "address_map_arm.h"
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
