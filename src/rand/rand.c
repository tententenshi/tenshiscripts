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
	double fromVal = 0;
	double toVal = 0xffff;
	int isHexMode = 0;
	int isDoubleMode = 0;
	int fractionDigit = 1;

	thisProgName = *argv;
	argc--;
	argv++;

	if (argc == 0) {
		usage(thisProgName);
		return 0;
	}
	length = strtoul(*argv, 0, 0);
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
			const char *pFromVal, *pToVal;
			char* pBuf = (char*)malloc(strlen(*argv) + 1);
			strcpy(pBuf, *argv);
			pBuf[pChar - *argv] = '\0';
			pFromVal = pBuf;
			pToVal = pBuf + (pChar - *argv) + 1;
			if (strchr(pFromVal, '.')) {
				int aTmpFracDigit = strlen(strchr(pFromVal, '.')) - 1;
				fractionDigit = (fractionDigit > aTmpFracDigit) ? fractionDigit : aTmpFracDigit;
				isDoubleMode = 1;
			}
			if (strchr(pToVal, '.')) {
				int aTmpFracDigit = strlen(strchr(pToVal, '.')) - 1;
				fractionDigit = (fractionDigit > aTmpFracDigit) ? fractionDigit : aTmpFracDigit;
				isDoubleMode = 1;
			}
			if ((strncmp(pFromVal, "0x", 2) == 0) || (strncmp(pFromVal, "0X", 2) == 0) || 
			    (strncmp(pToVal, "0x", 2) == 0) || (strncmp(pToVal, "0X", 2) == 0)) {
				isHexMode = 1;
			}

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
	for (i = 0; i < length; i++) {
		unsigned long aRandVal = genrand_int32();
		if (isDoubleMode) {
#if 0
			double aRandDouble = (double)aRandVal * (toVal - fromVal) / 4294967295.0 + fromVal;
#else
			double aRange = toVal - fromVal;
			int aBase = 1, i;
			int aIntRange;
			double aRandDouble;
			for (i = 0; i < fractionDigit; i++) {
				aBase = aBase * 10;
			}
			aIntRange = (int)(aRange * aBase);
			aRandDouble = (double)(aRandVal % (aIntRange + 1)) / aBase + fromVal;
#endif
			if (outFileName != 0) {
				if (isBinaryMode) {
					fwrite(&aRandDouble, sizeof(double), 1, objFp);
				} else {
					fprintf(objFp, "%.*f\n", fractionDigit, aRandDouble);
				}
			} else {
				printf("%.*f\n", fractionDigit, aRandDouble);
			}
		} else {
			long aRange = (long)(toVal - fromVal);
			long aRangedRandVal = aRandVal % (aRange + 1) + (long)fromVal;
			if (outFileName != 0) {
				if (isBinaryMode) {
					if ((fabs(fromVal) > 0xffff) || (fabs(toVal) > 0xffff)) {
						fwrite(&aRangedRandVal, sizeof(long), 1, objFp);
					} else if ((fabs(fromVal) > 0xff) || (fabs(toVal) > 0xff)) {
						short aShortRandVal = (short)aRangedRandVal;
						fwrite(&aShortRandVal, sizeof(short), 1, objFp);
					} else {
						char aCharRandVal = (char)aRangedRandVal;
						fwrite(&aCharRandVal, sizeof(char), 1, objFp);
					}
				} else if (isHexMode) {
					if ((fabs(fromVal) > 0xffff) || (fabs(toVal) > 0xffff)) {
						fprintf(objFp, "0x%08x\n", aRangedRandVal);
					} else if ((fabs(fromVal) > 0xff) || (fabs(toVal) > 0xff)) {
						fprintf(objFp, "0x%04x\n", (aRangedRandVal & 0xffff));
					} else {
						fprintf(objFp, "0x%02x\n", (aRangedRandVal & 0xff));
					}
				} else {
					fprintf(objFp, "%d\n", aRangedRandVal);
				}
			} else {
				if (isHexMode) {
					if ((fabs(fromVal) > 0xffff) || (fabs(toVal) > 0xffff)) {
						printf("0x%08x\n", aRangedRandVal);
					} else if ((fabs(fromVal) > 0xff) || (fabs(toVal) > 0xff)) {
						printf("0x%04x\n", (aRangedRandVal & 0xffff));
					} else {
						printf("0x%02x\n", (aRangedRandVal & 0xff));
					}
				} else {
					printf("%d\n", aRangedRandVal);
				}
			}
		}

	}

	if (outFileName != 0) {
		fclose(objFp);
	}
}
