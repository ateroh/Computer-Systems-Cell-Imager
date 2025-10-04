#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "cbmp.h"

// Funktionsdeklareringer fra functions.c
void invert(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]);


void convert_to_greyscale(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]);

void binary_threshold(unsigned int threshold, unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH]);

int basic_erosion(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH], unsigned int threshold, int coordinate_x[], int coordinate_y[], int capacity);

int detect_spots(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], int coordinate_x[], int coordinate_y[], int total_detections, int capacity);

unsigned int otsu_method(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]);

void black_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int x, int y);
void light_beige_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int x, int y);
void dark_beige_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int x, int y);
void brown_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int x, int y);
void green_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int x, int y);
void dark_green_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int x, int y);
void red_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int x, int y);
void dark_red_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int x, int y);
void light_blue_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int x, int y);
void blue_teemo_color(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int x, int y);

void generate_output_image(
    unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS],
    unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS],
    int detections,
    int coordinate_x[],
    int coordinate_y[]);

#endif // FUNCTIONS_H
