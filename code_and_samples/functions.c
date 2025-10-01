# include "cbmp.h"
# include <stdio.h>
# include "functions.h"


#define THRESHOLD 90

unsigned char temp_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];

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
                output_image[x][y][c] = (r + g + b) / 3;
            }
        }
    }
}



// Function to threshold an image Step 3
void binary_threshold(unsigned int threshold, unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS],
                      unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {
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
int basic_erosion(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS],
                   unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned int threshold, 
                   int coordinate_x[], int coordinate_y[], int capacity) {
    binary_threshold(threshold, input_image, output_image);


    int total_detections = 0;
    int eroded_cells = 1;
    int erosion_pass = 0;
    
    // Eroded all borders so that we can detect cells that are half off the image
    for (int j = 0; j < BMP_CHANNELS; j++) {
        for (int i = 0; i < BMP_WIDTH; i++) {
            output_image[0][i][j] = 0;
            output_image[BMP_HEIGTH-1][i][j] = 0;
            output_image[i][0][j] = 0;
            output_image[i][BMP_WIDTH-1][j] = 0;
        }
    }
    // erosion pass used to check after # erosions
    while (eroded_cells && erosion_pass < 3) {
        eroded_cells = 0; // incase of only one erosion occurs
        erosion_pass++;

        for (int x = 0; x < BMP_WIDTH; x++) {
            for (int y = 0; y < BMP_HEIGTH; y++) {
                for (int c = 0; c < BMP_CHANNELS; c++) {
                    temp_image[x][y][c] = output_image[x][y][c];
                }
            }
        }

        

        for (int x = 1; x < BMP_WIDTH - 1; x++) {
            // we go from 1 to width-1 to avoid borders
            for (int y = 1; y < BMP_HEIGTH - 1; y++) {
                if (temp_image[x][y][2] == 255) {
                    //if the center is white we continue
                    if (temp_image[x - 1][y][2] == 255
                        && temp_image[x + 1][y][2] == 255
                        && temp_image[x][y - 1][2] == 255
                        && temp_image[x][y + 1][2] == 255) {
                        // keep white (do nothing to output)
                    } else {
                        for (int c = 0; c < BMP_CHANNELS; c++) {
                            output_image[x][y][c] = 0;
                        }
                        eroded_cells = 1; //erosion occured
                    }
                }
            }
        }
        if (total_detections > capacity) {
            total_detections = capacity;
        } else {
        total_detections += detect_spots(output_image, coordinate_x, coordinate_y, total_detections, capacity);
        }
    }
    
    // this code is used for checking after each erosion. reverts back to output image for generate_image in main
    
    for (int x = 0; x < BMP_WIDTH; x++) {
            for (int y = 0; y < BMP_HEIGTH; y++) {
                for (int c = 0; c < BMP_CHANNELS; c++) {
                    output_image[x][y][c] = temp_image[x][y][c];
                }
            }
        }
    
    return total_detections;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                         STEP 5: DETECT SPOTS                                   //
////////////////////////////////////////////////////////////////////////////////////////////////////
int detect_spots(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int coordinate_x[], int coordinate_y[], int total_detections, int capacity) {
    // Konstanter for vinduet
    int capture = 6; // halv størrelse for 12x12 (capture)
    int exclusion_frame = capture + 1; // +1 pixel ring (exclusion frame)
    int detections = 0;

    if (total_detections >= capacity) return 0;

    // Loop kun hvor hele 14x14 (Exclusion frame) er inde i billedet
    for (int x = exclusion_frame; x < BMP_WIDTH - exclusion_frame; x++) {
        for (int y = exclusion_frame; y < BMP_HEIGTH - exclusion_frame; y++) {
            
            // Kræv at center-pixel er hvid for at undgå at tælle støjlige nabopixels
            if (input_image[x][y][0] != 255 && input_image[x][y][1] != 255 && input_image[x][y][2] != 255 && input_image[x][y][3] != 255 ) continue;


            /*      tester lige uden min_capture_whites

            //  Tæl hvide pixels i 12x12 capture-området
            int white_count = 0;
            for (int dx = -capture; dx <= capture - 1; dx++) {
                for (int dy = -capture; dy <= capture - 1; dy++) {
                    if (input_image[x + dx][y + dy][2] == 255) {
                        white_count++;
                    }
                }
            }
            if (white_count < MIN_CAPTURE_WHITES) {
                continue; // for få hvide pixels -> sandsynligvis støj
            }
            */

            // 1 means exclusion zone is black (free of cells)
            int ring_is_black = 1;

            // Øverste og nederste ramme for Exclusion frame
            for (int dx = -exclusion_frame; dx <= exclusion_frame && ring_is_black; dx++) {
                if (input_image[x + dx][y - exclusion_frame][2] == 255
                    || input_image[x + dx][y + exclusion_frame][2] == 255 )  {
                    ring_is_black = 0;
                    break;
                }
            }
            // Venstre og højre ramme af exclusion frame
            for (int dy = -exclusion_frame; dy <= exclusion_frame && ring_is_black; dy++) {
                if (input_image[x - exclusion_frame][y + dy][2] == 255
                    || input_image[x + exclusion_frame][y + dy][2] == 255) {
                    ring_is_black = 0;
                    break;
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
            for (int dx = -exclusion_frame; dx <= exclusion_frame; dx++) {
                for (int dy = -exclusion_frame; dy <= exclusion_frame; dy++) {
                    for (int c = 0; c < BMP_CHANNELS; c++) {
                        input_image[x + dx][y + dy][c] = 0;
                    }
                }
            }
        }
    }
    return detections;
}

unsigned int otsu_method(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {
    unsigned long histogram[256];
    int pixel_value;
    const unsigned long total = BMP_HEIGTH * BMP_WIDTH;
    
    for (int i = 0; i < 256 ; i++) histogram[i] = 0;
    
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
    unsigned int sumB = 0.0;
    // foreground: weight: sum
    unsigned int wF = 0; 
    unsigned int sumF = 0.0;

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

    // Kopier originalt billede
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y< BMP_HEIGTH; y++) {
            for (int c = 0; c < BMP_CHANNELS; c++) {
                output_image[x][y][c] = input_image[x][y][c];
            }
        }
    }
        // Tegn et rødt kryds i centeret af hver fundne celle

        for (int i = 0; i < detections; i++) {
            int cx = coordinate_x[i];
            int cy = coordinate_y[i];

            // Horizontal
            for (int dx = -cross_length; dx <= cross_length; dx++) {
                int x = cx + dx;
                int y = cy;
                if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                    output_image[x][y][0] = 255;
                    output_image[x][y][1] = 0;
                    output_image[x][y][2] = 0;
                }
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
            }
        }
}