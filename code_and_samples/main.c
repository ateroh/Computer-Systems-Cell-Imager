//To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99 gcc cbmp.c main.c functions.c -o main.out -std=c99

//To run (linux/mac): ./main.out example.bmp example_inv.bmp

//To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
//TO compile (win) for newer: gcc cbmp.c main.c functions.c -o main.exe -std=c99
//To run (win): main.exe example.bmp example_inv.bmp

#include <stdlib.h>
#include <stdio.h>
#include "cbmp.h"
#include "functions.h"

#define THRESHOLD 90

// Globale billede-arrays
unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];



//Main function
int main(int argc, char** argv)
{
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

  //Load image from file
  read_bitmap(argv[1], input_image);

  printf("Loaded image!\n");

  //Run inversion
  // invert(input_image,output_image);

  //printf("Inverted image!\n");

  
  // convert_to_greyscale(input_image, output_image);
  //printf("Converted to greyscale!\n");
  

  binary_threshold(THRESHOLD, input_image, output_image);
  
  //Save image to file
  write_bitmap(output_image, argv[2]);
  //printf("Saved image!\n");

  

  printf("Done!\n");
  return 0;
}
