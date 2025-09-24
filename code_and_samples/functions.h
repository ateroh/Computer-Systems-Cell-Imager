#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "cbmp.h"

// Funktionsdeklareringer fra functions.c
void invert(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]);


void convert_to_greyscale(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]);

void binary_threshold(int threshold, unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]);

int basic_erosion(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int coordinate_x[], int coordinate_y[]);

int detect_spots(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int coordinate_x[], int coordinate_y[]);
#endif // FUNCTIONS_H
