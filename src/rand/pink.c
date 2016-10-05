#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mersenne_twister.h"

#define	GAIN	3.0

void usage(const unsigned char* thisProgName)
{
	printf("usage: %s 10000[sample] [seed value] [-o outFileName]\n", thisProgName);
	printf("      options:\n");
	printf("              o: speciy output file (*.bin --> output binary, else output text)\n");
}

/* source from
http://www.finetune.co.jp/~lyuka/technote/pinknoise/
*/


#define MAX_Z 8

double  z[MAX_Z];
double  k[MAX_Z];

double pinkfilter(double in) {
	extern double   z[MAX_Z];
	extern double   k[MAX_Z];
	static double   t = 0.0;
	double          q;
	int             i;

	q = in;
	for (i = 0; i < MAX_Z; i++) {
		z[i] = (q * k[i] + z[i] * (1.0 - k[i]));
		q = (q + z[i]) * 0.5;
	}
	t = 0.75 * q + 0.25 * t;  /* add 1st order LPF */
	return t;
}


/* source from
http://www.firstpr.com.au/dsp/pink-noise/#Filtering
*/

double pinkfilter2(double in) {
	static double b0 = 0.0;
	static double b1 = 0.0;
	static double b2 = 0.0;
	static double b3 = 0.0;
	static double b4 = 0.0;
	static double b5 = 0.0;
	static double b6 = 0.0;
	double pink;

	b0 = 0.99886 * b0 + in * 0.0555179;
	b1 = 0.99332 * b1 + in * 0.0750759;
	b2 = 0.96900 * b2 + in * 0.1538520;
	b3 = 0.86650 * b3 + in * 0.3104856;
	b4 = 0.55000 * b4 + in * 0.5329522;
	b5 = -0.7616 * b5 - in * 0.0168980;
	pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + in * 0.5362;
	b6 = in * 0.115926; 

	return pink / 69.054393845962;
}

void init_pink() {
	extern double   z[MAX_Z];
	extern double   k[MAX_Z];
	int             i;

	for (i = 0; i < MAX_Z; i++) { z[i] = 0; }
	k[MAX_Z - 1] = 0.5;
	for (i = MAX_Z - 1; i > 0; i--) {
		k[i - 1] = k[i] * 0.25;
	}
}

int main(int argc, char **argv)
{
	const char * thisProgName;
	unsigned long length;
	unsigned long seed;
	int isSeedSpecified = 0;
	const char * outFileName = 0;
	FILE *objFp;
	int i;
	int isBinaryMode = 0;

	thisProgName = *argv;
	argc--;
	argv++;

	if (argc == 0) {
		usage(thisProgName);
		return;
	}
	length = strtoul(*argv, 0, 0);
	argc--, argv++;

	for (; 0 < argc; argc--, argv++) {
		if (argv[0][0] == '-') {
			switch (argv[0][1]) {
			case 'o':
				argc--, argv++;
				if (argc == 0) {
					printf("output file not specified\n");
					usage(thisProgName);
					return;
				}
				outFileName = *argv;
				break;
			default:
				break;
			}
		} else {
			seed = strtoul(*argv++, 0, 0);
			isSeedSpecified = 1;
		}
	}

	if (!isSeedSpecified) {
		seed = (unsigned long)time(0);
	}

	if (outFileName != 0) {
		objFp = fopen(outFileName, "w");

		if (objFp == 0) {
			printf("cannot open %s\n", outFileName);
			return;
		}
		{
			const char* extention = strrchr(outFileName, '.') + 1;
			if (strcmp(extention, "bin") == 0) {
				isBinaryMode = 1;
			}
		}
	}

	init_genrand(seed);
	init_pink();

	for (i = 0; i < length; i++) {
		double aRandVal = genrand_real2() * 2.0 - 1.0;
//		double aPinkVal = pinkfilter(aRandVal);
		double aPinkVal = pinkfilter2(aRandVal);
//		printf("%6.3f\n", aPinkVal);
		unsigned short aWord = (unsigned short)(aPinkVal * GAIN * 0x7fff);
		if (outFileName != 0) {
			if (isBinaryMode) {
				fputc((aWord & 0x00ff) >> 0, objFp);
				fputc((aWord & 0xff00) >> 8, objFp);
			} else {
				fprintf(objFp, "%hu\n", aWord);
			}
		} else {
			printf("%hu\n", aWord);
		}
	}

	if (outFileName != 0) {
		fclose(objFp);
	}
}
