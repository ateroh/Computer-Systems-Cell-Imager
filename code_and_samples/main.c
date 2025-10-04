//To compile (linux/mac): gcc cbmp.c main.c functions.c -o main.out -std=c99

// gcc -fsanitize=address -g cbmp.c main.c functions.c -o memory_test -std=c99


//To run (linux/mac): ./main.out example.bmp example_inv.bmp

//To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
//TO compile (win) for newer: gcc cbmp.c main.c functions.c -o main.exe -std=c99
//To compile (win): gcc cbmp.c main.c functions.c -o main.exe -std=c99
//To run (win): .\main.exe example.bmp example_inv.bmp
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "cbmp.h"
#include "functions.h"

#define THRESHOLD 90
#define MAX_CELLS 310

// Globale billede-arrays
unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH];
unsigned int threshold;


//Main function
int main(int argc, char** argv)
{
  int coordinate_x[MAX_CELLS];
  int coordinate_y[MAX_CELLS];
  clock_t start, end;

  //argc counts how may arguments are passed
  //argv[0] is a string with the name of the program
  //argv[1] is the first command line argument (input image)
  //argv[2] is the second command line argument (output image)

  //Checking that 2 arguments are passed
  if (argc != 3)
  {
      fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
      exit(1);
  }
  printf("#---------------------------#\n");
  printf("|   Cell Detection Program  |\n");
  printf("|---------------------------|\n");

  //printf("Example program - 02132 - A1\n");
  // Start timer
  start = clock();


  //Load image from file
  read_bitmap(argv[1], input_image);

  printf("Loaded image!\n");

  //Run inversion
  //invert(input_image,output_image);

  //printf("Inverted image!\n");

  convert_to_greyscale(input_image, output_image);
  printf("Converted to greyscale!\n");

  //binary_threshold(THRESHOLD, input_image, output_image);
  
  threshold = otsu_method(output_image)-10;
  printf("Otsu calculated threshold: %u (original was 90)\n", threshold);

  //int cells = basic_erosion(input_image, binary_image, threshold, coordinate_x, coordinate_y, MAX_CELLS);
  int cells = detect_cells_distance_transform(input_image, threshold, coordinate_x, coordinate_y, MAX_CELLS);
  printf("Image eroded!\n");

  printf("Antal celler: %d\n", cells);


  /*for (int i = 0; i < cells && i < MAX_CELLS; i++) {
    printf("Cell %d: (%d, %d)\n", i + 1, coordinate_x[i], coordinate_y[i]);
  } */

  // to test per erosion change input_image to output_image and edit basic_erosion
  generate_output_image(input_image, output_image, cells, coordinate_x, coordinate_y);

  //Save image to file
  write_bitmap(output_image, argv[2]);
  printf("Saved image!\n");


  printf("Done!\n");


  // Udregner tid brugt fra CPU https://stackoverflow.com/questions/5248915/execution-time-of-c-program

  end = clock();
  double time_spent = ((double) (end - start)) / CLOCKS_PER_SEC;

  printf("Time used: %f seconds\n", time_spent);
  return 0;
}
