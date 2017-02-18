#include"bitmap.h"
#include<stdlib.h>
#include<stdio.h>

int main(int argc, char *argv[])
{
	if(argc != 3){
		fprintf(stderr, "Usage: program <inputfile> <outputfile>\n");
		exit(1);
	}

	Image *colorimg, *Rimg, *Gimg, *Bimg;

	if((colorimg = Read_Bmp(argv[1])) == NULL){
		exit(1);
	}

	if((Rimg = Create_Image(colorimg->width, colorimg->height)) == NULL){
		exit(1);
	}
	if((Gimg = Create_Image(colorimg->width, colorimg->height)) == NULL){
		exit(1);
	}
	if((Bimg = Create_Image(colorimg->width, colorimg->height)) == NULL){
		exit(1);
	}

	RGBDivision(colorimg, Rimg, Gimg, Bimg);

	Errordiffusion(Rimg);
	Errordiffusion(Gimg);
	Errordiffusion(Bimg);

	RGBCombination(Rimg, Gimg, Bimg, colorimg);
	
	if(Write_Bmp(argv[2], colorimg)){
		exit(1);
	}

	Free_Image(Rimg);
	Free_Image(Gimg);
	Free_Image(Bimg);


	Free_Image(colorimg);

	return 0;
}

