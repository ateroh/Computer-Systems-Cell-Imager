# include "cbmp.h"
# include <stdio.h>
# include "functions.h"


#define THRESHOLD 90

//Function to invert pixels of an image (negative)
void invert(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]){
    for (int x = 0; x < BMP_WIDTH; x++)
    {
        for (int y = 0; y < BMP_HEIGTH; y++)
        {
            for (int c = 0; c < BMP_CHANNELS; c++)
            {
                output_image[x][y][c] = 255 - input_image[x][y][c];
            }
        }
    }
}

// Function to convert an image to greyscale Step 2
void convert_to_greyscale(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {
    for (int x=0; x < BMP_WIDTH; x++) {
        for (int y=0; y < BMP_HEIGTH; y++) {
            for (int c=0; c < BMP_CHANNELS; c++) {
                unsigned char r = input_image[x][y][0];
                unsigned char g = input_image[x][y][1];
                unsigned char b = input_image[x][y][2];
                output_image[x][y][c] = (r + g + b)/3;
            }
        }
    }
}

// Function to threshold an image Step 3
void binary_threshold(int threshold, unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {
    for (int i = 0; i < BMP_WIDTH; i++) {
        for (int j = 0; j < BMP_HEIGTH; j++) {
            for (int c = 0; c < BMP_CHANNELS; c++) {
                unsigned char post_threshold_value;

                if (input_image[i][j][0] > threshold) {
                    post_threshold_value = 255;
                } else {
                    post_threshold_value = 0;
                }
                output_image[i][j][c] = post_threshold_value;
            }
        }
    }
}

//Function that erodes image (basic) Step 4
void basic_erosion(int threshold, unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {
    binary_threshold(threshold, input_image, output_image);

    int eroded_cells = 1;
    unsigned char temp_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];

    //Creating a temporaray 3d array so that we can run multiple erosions
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            for (int c = 0; c < BMP_CHANNELS; c++) {
                temp_image[x][y][c] = output_image[x][y][c];
            }
        }   
    }
    while (eroded_cells) {
        eroded_cells = 0; // incase of only one erosion needed
        for (int x = 1; x < BMP_WIDTH-1; x++) {         // we go from 1 to width-1 to avoid borders 
            for (int y = 1; y < BMP_HEIGTH-1; y++) {
                if (temp_image[x][y][2] == 255 
                    && temp_image[x-1][y][2] == 255 
                    && temp_image[x+1][y][2] == 255 
                    && temp_image[x][y-1][2] == 255 
                    && temp_image[x][y+1][2] == 255) {    
                    // checks all orthogonally adjacent cell to see if they are also white. If so, do nothing (keep alive)
                } else {
                    for (int c = 0; c < BMP_CHANNELS; c++) {
                        output_image[x][y][c] = 0;   

                        }
                        eroded_cells = 1;   // cells were eroded
                    }
                }
            }
    }
}

// Step 5 detect spots in image

int detect_spots(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {
}
