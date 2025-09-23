# include "cbmp.h"
# include <stdio.h>
# include "functions.h"


#define THRESHOLD 90

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
////////////////////////////////////////////////////////////////////////////////////////////////////
//                                         STEP 2: Convert to gray-scale                          //
////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                         STEP 3: Apply binary threshold                         //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Function to threshold an image Step 3
void binary_threshold(int threshold, unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS],
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

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                         STEP 4: Erode image                                     //
////////////////////////////////////////////////////////////////////////////////////////////////////
//Function that erodes image (basic) Step 4
void basic_erosion(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS],
                   unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {
    binary_threshold(THRESHOLD, input_image, output_image);

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
                    // Center was white: erode unless all 4-neighbors are also white
                    if (temp_image[x - 1][y][2] == 255
                        && temp_image[x + 1][y][2] == 255
                        && temp_image[x][y - 1][2] == 255
                        && temp_image[x][y + 1][2] == 255) {
                        // keep white (no write needed since output already holds current state)
                    } else {
                        for (int c = 0; c < BMP_CHANNELS; c++) {
                            output_image[x][y][c] = 0;
                        }
                        eroded_cells = 1; // a white pixel was eroded
                    }
                } else {
                    // Center already black: leave as is and do NOT set eroded_cells
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                         STEP 5: DETECT SPOTS                                    //
////////////////////////////////////////////////////////////////////////////////////////////////////
int detect_spots(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {
    // Konstanter for vinduet
    const int capture = 6; // halv størrelse for 12x12 (capture)
    const int exclusion_frame = capture + 1; // +1 pixel ring (exclusion frame)

    int detections = 0;

    // Loop kun hvor hele 14x14 (Exclusion frame) er inde i billedet
    for (int x = exclusion_frame; x < BMP_WIDTH - exclusion_frame; x++) {
        for (int y = exclusion_frame; y < BMP_HEIGTH - exclusion_frame; y++) {
            // 1) Find mindst én hvid pixel i 12x12 capture-området
            int has_white_in_capture = 0;
            for (int dx = -capture; dx <= capture - 1 && !has_white_in_capture; dx++) {
                for (int dy = -capture; dy <= capture - 1; dy++) {
                    if (input_image[x + dx][y + dy][2] == 255) {
                        has_white_in_capture = 1;
                        break;
                    }
                }
            }
            if (!has_white_in_capture) {
                continue; // intet at fange omkring dette center
            }

            int ring_is_black = 1;

            // Øverste og nederste ramme for Exclusion frame
            for (int dx = -exclusion_frame; dx <= exclusion_frame && ring_is_black; dx++) {
                if (input_image[x + dx][y - exclusion_frame][2] == 255 || input_image[x + dx][y + exclusion_frame][2]) {
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
            if (!ring_is_black) {
                continue; // intet at fange omkring dette center
            }
            // Registrer detektion og sætter 12x12 til sort
            detections++;

            for (int dx = -capture; dx <= capture - 1; dx++) {
                for (int dy = -capture; dy <= capture - 1; dy++) {
                    for (int c = 0; c < BMP_CHANNELS; c++) {
                        input_image[x + dx][y + dy][c] = 0;
                    }
                }
            }
        }
    }
    return detections;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                         STEP 6: Generate output image                          //
////////////////////////////////////////////////////////////////////////////////////////////////////
