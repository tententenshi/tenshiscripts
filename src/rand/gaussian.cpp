#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "mersenne_twister.h"

void usage(const char* thisProgName)
{
	printf("usage: %s 10000[sample] [seed value] [from_val-to_val] [-o outFileName]\n", thisProgName);
	printf("      options:\n");
	printf("              o: speciy output file (*.bin --> output binary, else output text)\n");
}

double rand_normal(double aRand1, double aRand2, double mu, double sigma)
{
	double z = sqrt(-2.0 * log(aRand1)) * sin(2.0 * M_PI * aRand2);
	return mu + sigma * z;
}

int main(int argc, char **argv)
{
	unsigned long seed;
	bool isSeedSpecified = false;
	bool isBinaryMode = false;
	const char * outFileName = 0;
	FILE *objFp;
	double fromVal = 0;
	double toVal = 0xffff;

	const char * thisProgName = *argv;
	argc--;
	argv++;

	if (argc == 0) {
		usage(thisProgName);
		return 0;
	}
	unsigned long length = strtoul(*argv, 0, 0);
	argc--, argv++;

	for (; 0 < argc; argc--, argv++) {
		const char* pChar;
		if ((argv[0][0] == '-') && (argv[0][1] == 'o')) {
			argc--, argv++;
			if (argc == 0) {
				printf("output file not specified\n");
				usage(thisProgName);
				return 0;
			}
			outFileName = *argv;
		} else if ((pChar = strchr(*argv + 1, '-')) != NULL) {
			char* pBuf = (char*)malloc(strlen(*argv) + 1);
			strcpy(pBuf, *argv);
			pBuf[pChar - *argv] = '\0';
			const char *pFromVal = pBuf;
			const char *pToVal = pBuf + (pChar - *argv) + 1;

			fromVal = atof(pFromVal);
			toVal = atof(pToVal);
			if (fromVal > toVal) {
				double tmpval = fromVal;	// swap
				fromVal = toVal;
				toVal = tmpval;
			}
			if (pBuf != NULL) {
				free(pBuf);
			}
		} else {
			seed = strtoul(*argv, 0, 0);
			isSeedSpecified = 1;
		}
	}

	if (!isSeedSpecified) {
		seed = (unsigned long)time(0);
	}

//	printf("range (%f - %f), digit(%d), isDouble(%d), hex(%d), seed(%d)\n", fromVal, toVal, fractionDigit, isDoubleMode, isHexMode, seed);

	if (outFileName != 0) {
		objFp = fopen(outFileName, "w");

		if (objFp == 0) {
			printf("cannot open %s\n", outFileName);
			exit(1);
		}
		{
			const char* extention = strrchr(outFileName, '.') + 1;
			if (strcmp(extention, "bin") == 0) {
				isBinaryMode = 1;
			}
		}
	}

	init_genrand(seed);
	for (int i = 0; i < length; i++) {
		double aNormRandVal = rand_normal(genrand_real3(), genrand_real3(), 0, 1);
		if (outFileName != 0) {
			if (isBinaryMode) {
				fwrite(&aNormRandVal, sizeof(double), 1, objFp);
			} else {
				fprintf(objFp, "%g\n", aNormRandVal);
			}
		} else {
			printf("%g\n", aNormRandVal);
		}
	}

	if (outFileName != 0) {
		fclose(objFp);
	}
}
