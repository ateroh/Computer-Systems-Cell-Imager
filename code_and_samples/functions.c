# include "cbmp.h"
# include <stdio.h>
# include "functions.h"
# include <string.h>
#include <time.h>

//included this for abs function
#include <stdlib.h>


#define THRESHOLD 90

clock_t start, end, start1, end1;

clock_t start, end, start1, end1;

unsigned char temp_image[BMP_WIDTH][BMP_HEIGTH];

//Function to invert pixels of an image (negative)
void invert(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS],
            unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            for (int c = 0; c < BMP_CHANNELS; c++) {
                output_image[x][y][c] = 255 - input_image[x][y][c];
            }
        }
    }
}

// Function to convert an image to greyscale Step 2
void convert_to_greyscale(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS],
                          unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            for (int c = 0; c < BMP_CHANNELS; c++) {
                unsigned char r = input_image[x][y][0];
                unsigned char g = input_image[x][y][1];
                unsigned char b = input_image[x][y][2];
                output_image[x][y][c] = ((r + g + b) * 85) >> 8; // (r+g+b) / 3
            }
        }
    }
}



// Function to threshold an image Step 3
void binary_threshold(unsigned int threshold, unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS],
                      unsigned char output_image[BMP_WIDTH][BMP_HEIGTH]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            output_image[x][y] = (input_image[x][y][0] > threshold) ? 255 : 0;
        }
    }
}


//Function that erodes image (basic) Step 4
int basic_erosion(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS],
                   unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH], unsigned int threshold, 
                   int coordinate_x[], int coordinate_y[], int capacity) {
    binary_threshold(threshold, input_image, binary_image);


    int total_detections = 0;
    int eroded_cells = 1;
    int erosion_pass = 0;
    
    // Eroded all borders so that we can detect cells that are half off the image
    
    for (int x = 0; x < BMP_WIDTH; x++) {
        binary_image[x][0] = 0;
        binary_image[x][BMP_HEIGTH - 1] = 0;
    }
    // Zero left and right columns (x = 0 and x = BMP_WIDTH-1)
    for (int y = 0; y < BMP_HEIGTH; y++) {
        binary_image[0][y] = 0;
        binary_image[BMP_WIDTH - 1][y] = 0;
    }

    

    // erosion pass used to check after # erosions
    while (eroded_cells > 0 /*&& erosion_pass < 3*/) {
        start = clock();
        eroded_cells = 0; // incase of only one erosion occurs
        //erosion_pass++;

        memcpy(temp_image, binary_image, sizeof(temp_image));

        

        for (int x = 1; x < BMP_WIDTH - 1; x++) {
            // we go from 1 to width-1 to avoid borders
            for (int y = 1; y < BMP_HEIGTH - 1; y++) {
                if (temp_image[x][y] == 255) {
                    //if the center is white we continue
                    if (temp_image[x - 1][y] == 255
                        && temp_image[x + 1][y] == 255
                        && temp_image[x][y - 1] == 255
                        && temp_image[x][y + 1] == 255

                        //diagonals - fixes certain problems but erosion becomes too aggresive
                        /*temp_image[x - 1][y - 1] == 255
                        && temp_image[x + 1][y - 1] == 255
                        && temp_image[x - 1][y + 1] == 255
                        && temp_image[x + 1][y + 1] == 255*/
                        
                        ) {
                        // keep white (do nothing to output)
                    } else {
                        for (int c = 0; c < BMP_CHANNELS; c++) {
                            binary_image[x][y] = 0;
                        }
                        eroded_cells = 1; //erosion occured
                    }
                }
            }
        }
        if (total_detections > capacity) {
            total_detections = capacity;
        } else {
            start1 = clock();
        total_detections += detect_spots(binary_image, coordinate_x, coordinate_y, total_detections, capacity);
            end1 = clock();
            double time_spent1 = ((double) (end1 - start1)) / CLOCKS_PER_SEC;
            printf("Time used on detecting spots: %f seconds\n", time_spent1);
        }
        end = clock();
        double time_spent = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("Time used on basic erosion: %f seconds\n", time_spent);

    }
    
    // this code is used for checking after each erosion. reverts back to output image for generate_image in main
    /*
    for (int x = 0; x < BMP_WIDTH; x++) {
            for (int y = 0; y < BMP_HEIGTH; y++) {
                
                output_image[x][y] = temp_image[x][y];
                
            }
        }
    */
    
    return total_detections;
}


// Detect Spot                                   
int detect_spots(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], int coordinate_x[], int coordinate_y[], int total_detections, int capacity) {
    // Konstanter for vinduet
    int capture = 6; // halv størrelse for 12x12 (capture)
    int exclusion_frame = capture + 1; // +1 pixel ring (exclusion frame)
    int detections = 0;

    if (total_detections >= capacity) return 0;

    // Loop kun hvor hele 14x14 (Exclusion frame) er inde i billedet
    for (int x = exclusion_frame; x < BMP_WIDTH - exclusion_frame; x++) {
        for (int y = exclusion_frame; y < BMP_HEIGTH - exclusion_frame; y++) {
            
            // Kræv at center-pixel er hvid for at undgå at tælle støjlige nabopixels
            if (input_image[x][y] != 255 ) continue;

            // 1 means exclusion zone is black (free of cells)
            int ring_is_black = 1;

            // Øverste og nederste ramme for Exclusion frame
            for (int dx = -exclusion_frame; dx <= exclusion_frame && ring_is_black; dx++) {
                if (input_image[x + dx][y - exclusion_frame] == 255 || input_image[x + dx][y + exclusion_frame] == 255 )  {
                    ring_is_black = 0;
                    
                }
            }
            // Venstre og højre ramme af exclusion frame
            for (int dy = -exclusion_frame; dy <= exclusion_frame && ring_is_black; dy++) {
                if (input_image[x - exclusion_frame][y + dy] == 255 || input_image[x + exclusion_frame][y + dy] == 255) {
                    ring_is_black = 0;
                    
                }
            }
            if (ring_is_black == 0) {
                continue; // intet at fange omkring dette center
            }

            


            if (total_detections + detections >= capacity) return detections;


            // Registrer detektion og sætter 12x12 til sort
            coordinate_x[total_detections + detections] = x;
            coordinate_y[total_detections + detections] = y;
            detections++;
            
            // erase cell
            int changed = 1;
            int erase_radius = 8; // How far to look for connected pixels
            
            for (int dx = -exclusion_frame; dx <= exclusion_frame; dx++) {
                for (int dy = -exclusion_frame; dy <= exclusion_frame; dy++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < BMP_WIDTH && ny >= 0 && ny < BMP_HEIGTH) {
                        input_image[nx][ny] = 0; 
                    }
                }
            }



            /*
            for (int dx = -exclusion_frame; dx <= exclusion_frame; dx++) {
                for (int dy = -exclusion_frame; dy <= exclusion_frame; dy++) {
                    for (int c = 0; c < BMP_CHANNELS; c++) {
                        input_image[x + dx][y + dy][c] = 0;
                    }
                }
            }
            */
            
            
            
            
        }

    }

    return detections;

}

unsigned int otsu_method(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {
    unsigned long histogram[256] = {0};
    int pixel_value;
    unsigned long total = BMP_HEIGTH * BMP_WIDTH;
    
     //for (int i = 0; i < 256 ; i++) histogram[i] = 0;
    memset(histogram, 0, sizeof(histogram));
    
    for (int i = 0; i < BMP_WIDTH; i++) {
        for (int j = 0; j < BMP_HEIGTH; j++) {
            pixel_value = input_image[i][j][2];
            histogram[pixel_value]++;
        }
    }
    unsigned int total_sum =  0;
    for (int i = 0; i < 256; i++) total_sum += i * histogram[i];

    // background: wieght, sum
    unsigned int wB = 0; 
    unsigned int sumB = 0;
    // foreground: weight: sum
    unsigned int wF = 0; 
    unsigned int sumF = 0;

    double muB;
    double muF;
    double temp_varainace;
    double variance=0;
    unsigned int optimal_threshold = 0;

    for (int i = 0; i < 256 ; i++)
    {
        wB +=histogram[i];
        if (wB==0) continue;
        wF = total - wB;
        if (wF==0) continue;
        sumB += i * histogram[i];

        sumF = total_sum - sumB;

        muB = sumB/(double)wB;
        muF = sumF/(double)wF;
        temp_varainace = (double)wB*(double)wF*(muB-muF)*(muB-muF);
        if (temp_varainace> variance) {
            variance = temp_varainace;
            optimal_threshold = i;
        }
        
    }
    
    return optimal_threshold;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//                                         STEP 6: Generate output image                          //
////////////////////////////////////////////////////////////////////////////////////////////////////
void generate_output_image(
    unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS],
    unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS],
    int detections,
    int coordinate_x[],
    int coordinate_y[]) {

    int cross_length = 10;

    /*Kopier originalt billede*/
    memcpy(output_image, input_image, sizeof(unsigned char) * BMP_WIDTH * BMP_HEIGTH * BMP_CHANNELS);


        // Tegn et rødt kryds i centeret af hver fundne celle

    for (int i = 0; i < detections; i++) {
        int cx = coordinate_x[i];
        int cy = coordinate_y[i];
            
        // Horizontal
        /*for (int dx = -cross_length; dx <= cross_length; dx++) {
            int x = cx + dx;
            int y = cy;
                
            output_image[x][y][0] = 255;
            output_image[x][y][1] = 0;
            output_image[x][y][2] = 0;
                
        }
        // Vertical
        for (int dy = -cross_length; dy <= cross_length; dy++) {
            int x = cx;
            int y = cy + dy;
            if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                output_image[x][y][0] = 255;
                output_image[x][y][1] = 0;
                output_image[x][y][2] = 0;
            }
        }*/

        int dx;
        int dy;
        int x;
        int y;
        for (dx = -8; dx <= -5; dx++) {
            x = cx+dx;
            y = cy - 8;
            if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                black_teemo_color(output_image, x, y);
            }
        }

        for (dx = -2; dx <= 1; dx++) {
            x = cx+dx;
            y = cy - 8;
            if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                black_teemo_color(output_image, x, y);
            }
        }

        for (dx = 4; dx <= 7; dx++) {
            x = cx+dx;
            y = cy - 8;
            if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                black_teemo_color(output_image, x, y);
            }
        }

        for (dy = -7; dy <= -5; dy++) {
            x = cx-8;
            y = cy+dy;
            if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                black_teemo_color(output_image, x, y);
            }
            x = cx+7;
            if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                black_teemo_color(output_image, x, y);
            }
        }

        black_teemo_color(output_image, cx-4, cy-7);
        black_teemo_color(output_image, cx-3, cy-7);
        black_teemo_color(output_image, cx+2, cy-7);
        black_teemo_color(output_image, cx+3, cy-7);
        black_teemo_color(output_image, cx+4, cy-6);
        black_teemo_color(output_image, cx+5, cy-5);
        black_teemo_color(output_image, cx+6, cy-4);
        black_teemo_color(output_image, cx-5, cy-6);
        black_teemo_color(output_image, cx-6, cy-5);
        black_teemo_color(output_image, cx-7, cy-4);

        for (dx = -7; dx <= 6; dx++) {
            x = cx + dx;
            y = cy - 3;
            if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                black_teemo_color(output_image, x, y);
            }   
        }

        black_teemo_color(output_image, cx-6, cy-2);
        black_teemo_color(output_image, cx+5, cy-2);
        black_teemo_color(output_image, cx-7, cy-1);
        black_teemo_color(output_image, cx+6, cy-1);
        black_teemo_color(output_image, cx-6, cy);
        black_teemo_color(output_image, cx+5, cy);
        black_teemo_color(output_image, cx-5, cy+1);
        black_teemo_color(output_image, cx+4, cy+1);

        for (dx = -4; dx <= -2; dx++) {
            x = cx + dx;
            y = cy + 2;
            if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                black_teemo_color(output_image, x, y);
            }   
        }

        for (dx = -1; dx <= 1; dx++) {
            x = cx + dx;
            y = cy + 3;
            if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                black_teemo_color(output_image, x, y);
            }   
        }

        for (dx = 2; dx <= 3; dx++) {
            x = cx + dx;
            y = cy + 2;
            if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                black_teemo_color(output_image, x, y);
            }   
        }

        black_teemo_color(output_image, cx-5, cy+3);
        black_teemo_color(output_image, cx+4, cy+3);
        black_teemo_color(output_image, cx-6, cy+4);
        black_teemo_color(output_image, cx-6, cy+5);
        black_teemo_color(output_image, cx-5, cy+6);
        black_teemo_color(output_image, cx+5, cy+4);
        black_teemo_color(output_image, cx+5, cy+5);
        black_teemo_color(output_image, cx+4, cy+6);

        for (dy = 4; dy <= 9; dy++) {
            x = cx - 4;
            y = cy + dy;
            if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                black_teemo_color(output_image, x, y);
            }   
            x = cx + 3;
            if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                black_teemo_color(output_image, x, y);
            }  
        }

        black_teemo_color(output_image, cx-3, cy+9);
        black_teemo_color(output_image, cx-2, cy+9);
        black_teemo_color(output_image, cx-1, cy+8);
        black_teemo_color(output_image, cx, cy+8);
        black_teemo_color(output_image, cx+1, cy+9);
        black_teemo_color(output_image, cx+2, cy+9);

        for (dx = -7; dx <= -5; dx++) {
            x = cx + dx;
            y = cy - 7;
            if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                light_beige_teemo_color(output_image, x, y);
            }   
        }
        light_beige_teemo_color(output_image, cx-7, cy-6);
        light_beige_teemo_color(output_image, cx-7, cy-5);

        for (dx = 4; dx <= 6; dx++) {
            x = cx + dx;
            y = cy - 7;
            if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                light_beige_teemo_color(output_image, x, y);
            }   
        }
        light_beige_teemo_color(output_image, cx+6, cy-6);
        light_beige_teemo_color(output_image, cx+6, cy-5);

        for (dx = -5; dx <= 4; dx++) {
            for (dy = -2; dy <= 0; dy++) {
                x = cx + dx;
                y = cy + dy;
                if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                    light_beige_teemo_color(output_image, x, y);
                }   
            }
        }
        light_beige_teemo_color(output_image, cx-6, cy-1);
        light_beige_teemo_color(output_image, cx+5, cy-1);
        for (dx = -4; dx <= 3; dx++) {
            x = cx + dx;
            y = cy + 1;
            if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                light_beige_teemo_color(output_image, x, y);
            }   
        }
        for (dx = -1; dx <= 1; dx++) {
            x = cx + dx;
            y = cy + 2;
            if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                light_beige_teemo_color(output_image, x, y);
            }   
        }

        light_beige_teemo_color(output_image, cx-3, cy+4);
        for (dx = -3; dx <= 2; dx++) {
            for (dy = 5; dy <= 6; dy++) {
                x = cx + dx;
                y = cy + dy;
                if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                    light_beige_teemo_color(output_image, x, y);
                }   
            }    
        }

        for (dx = -3; dx <= 3; dx++) {
            for (dy = -2; dy <= 0; dy++) {
                x = cx + dx;
                y = cy + dy;
                if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                    dark_beige_teemo_color(output_image, x, y);
                }   
            }
        }
        for (dx = -3; dx <= 0; dx++) {
            x = cx + dx;
            y = cy + 7;
            if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                dark_beige_teemo_color(output_image, x, y);
            }   
        }

        brown_teemo_color(output_image, cx-6, cy-6);
        brown_teemo_color(output_image, cx+5, cy-6);
        brown_teemo_color(output_image, cx, cy);
        brown_teemo_color(output_image, cx+1, cy+7);
        brown_teemo_color(output_image, cx+2, cy+7);

        black_teemo_color(output_image, cx-3, cy-1);
        black_teemo_color(output_image, cx-1, cy-1);
        black_teemo_color(output_image, cx+1, cy-1);
        black_teemo_color(output_image, cx+3, cy-1);
        black_teemo_color(output_image, cx-2, cy-2);
        black_teemo_color(output_image, cx+2, cy-2);

        green_teemo_color(output_image, cx-6, cy-4);
        green_teemo_color(output_image, cx-5, cy-4);
        green_teemo_color(output_image, cx-5, cy-5);
        green_teemo_color(output_image, cx-4, cy-6);
        green_teemo_color(output_image, cx-2, cy-7);
        green_teemo_color(output_image, cx-1, cy-6);
        green_teemo_color(output_image, cx, cy-6);
        green_teemo_color(output_image, cx+1, cy-6);

        green_teemo_color(output_image, cx-5, cy+4);
        green_teemo_color(output_image, cx-5, cy+5);
        green_teemo_color(output_image, cx-3, cy+8);
        green_teemo_color(output_image, cx-2, cy+8);

        dark_green_teemo_color(output_image, cx-1, cy-7);
        dark_green_teemo_color(output_image, cx, cy-7);
        dark_green_teemo_color(output_image, cx+1, cy-7);
        dark_green_teemo_color(output_image, cx, cy-4);
        dark_green_teemo_color(output_image, cx+5, cy-4);

        dark_green_teemo_color(output_image, cx+4, cy+4);
        dark_green_teemo_color(output_image, cx+4, cy+5);
        dark_green_teemo_color(output_image, cx+1, cy+8);
        dark_green_teemo_color(output_image, cx+2, cy+8);

        red_teemo_color(output_image, cx-4, cy-4);
        red_teemo_color(output_image, cx-4, cy-5);
        red_teemo_color(output_image, cx-3, cy-6);
        red_teemo_color(output_image, cx+1, cy-4);
        red_teemo_color(output_image, cx+1, cy-5);
        red_teemo_color(output_image, cx+2, cy-6);

        red_teemo_color(output_image, cx-4, cy+3);
        red_teemo_color(output_image, cx-2, cy+3);
        red_teemo_color(output_image, cx+2, cy+3);
        red_teemo_color(output_image, cx-1, cy+4);
        red_teemo_color(output_image, cx+1, cy+4);

        dark_red_teemo_color(output_image, cx-3, cy-4);
        dark_red_teemo_color(output_image, cx-2, cy-4);
        dark_red_teemo_color(output_image, cx-1, cy-4);
        dark_red_teemo_color(output_image, cx-1, cy-5);
        dark_red_teemo_color(output_image, cx-2, cy-6);
        dark_red_teemo_color(output_image, cx, cy-5);
        dark_red_teemo_color(output_image, cx+2, cy-4);
        dark_red_teemo_color(output_image, cx+3, cy-4);
        dark_red_teemo_color(output_image, cx+4, cy-4);
        dark_red_teemo_color(output_image, cx+4, cy-5);
        dark_red_teemo_color(output_image, cx+3, cy-6);

        dark_red_teemo_color(output_image, cx-3, cy+3);
        dark_red_teemo_color(output_image, cx+3, cy+3);
        dark_red_teemo_color(output_image, cx-2, cy+4);
        dark_red_teemo_color(output_image, cx, cy+4);
        dark_red_teemo_color(output_image, cx+2, cy+4);

        light_blue_teemo_color(output_image, cx-3, cy-5);
        light_blue_teemo_color(output_image, cx+2, cy-5);

        light_blue_teemo_color(output_image, cx, cy-9);
        light_blue_teemo_color(output_image, cx-1, cy-11);

        blue_teemo_color(output_image, cx-2, cy-5);
        blue_teemo_color(output_image, cx+3, cy-5);

        blue_teemo_color(output_image, cx-1, cy-9);
        blue_teemo_color(output_image, cx, cy-10);
        blue_teemo_color(output_image, cx+1, cy-10);
        blue_teemo_color(output_image, cx, cy-11);
    }
}

void black_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int  x, int  y) {
    output_image[x][y][0] = 0;
    output_image[x][y][1] = 0;
    output_image[x][y][2] = 0;
}

void light_beige_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int  x, int  y) {
    output_image[x][y][0] = 224;
    output_image[x][y][1] = 171;
    output_image[x][y][2] = 139;
}

void dark_beige_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int  x, int  y) {
    output_image[x][y][0] = 198;
    output_image[x][y][1] = 136;
    output_image[x][y][2] = 99;
}

void brown_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int  x, int  y) {
    output_image[x][y][0] = 176;
    output_image[x][y][1] = 108;
    output_image[x][y][2] = 73;
}

void green_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int  x, int  y) {
    output_image[x][y][0] = 14;
    output_image[x][y][1] = 255;
    output_image[x][y][2] = 0;
}

void dark_green_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int  x, int  y) {
    output_image[x][y][0] = 31;
    output_image[x][y][1] = 198;
    output_image[x][y][2] = 0;
}

void red_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int  x, int  y) {
    output_image[x][y][0] = 255;
    output_image[x][y][1] = 0;
    output_image[x][y][2] = 0;
}

void dark_red_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int  x, int  y) {
    output_image[x][y][0] = 200;
    output_image[x][y][1] = 0;
    output_image[x][y][2] = 5;
}

void light_blue_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int  x, int  y) {
    output_image[x][y][0] = 135;
    output_image[x][y][1] = 206;
    output_image[x][y][2] = 251;
}

void blue_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int  x, int  y) {
    output_image[x][y][0] = 101;
    output_image[x][y][1] = 155;
    output_image[x][y][2] = 223;
}
