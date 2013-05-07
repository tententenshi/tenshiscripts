#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>

#define PI (4 * atan2(1,1))
static const int FS = 44100;
static const int PTR_MAX = (int)(0.3 * 44100);

void ReadFile(FILE* fp, FILE* fp2, int fsize);

double rms(unsigned short aVal) {
	double fVal = (double)aVal / 0x8000;
	double rms = fVal * PI / 2;	/* average --> rms */

	double dB = (rms <= 0) ? -99 : 20 * log10(rms);
	return dB;
}

static char buf[30];
static int TOC;
static int TOCArray[99];
static const char* timeformat(int aVal) {
	if (!TOC) {
		int minutes = (aVal / 60 / FS);
		int seconds = (aVal / FS) % 60;
		int sectors = ((aVal % FS) / 588);

		sprintf(buf, "%3d:%2d:%2d", minutes, seconds, sectors);
		return buf;
	} else {
		int tmptrack = 1;
		int tmpval = aVal;
		int minutes, seconds, sectors;
		int i;
		for (i = 1; (i < 99) && (TOCArray[i] > 0); i++) {
			if (TOCArray[i] > aVal) {
				break;
			} else {
				tmptrack = i + 1;
				tmpval = aVal - TOCArray[i];
			}
		}
		minutes = (tmpval / 60 / FS);
		seconds = (tmpval / FS) % 60;
		sectors = ((tmpval % FS) / 588);

		sprintf(buf, "Trk%2d-%2d:%2d:%2d (%4dsec)", tmptrack, minutes, seconds, sectors, aVal / FS);
		return buf;
	}
}

static int GetFileSize(char* name)
{
	struct stat stbuf;
	stat(name, &stbuf);

	return stbuf.st_size;
}

int main(int argc, char* argv[])
{
	char* infile = *++argv;
	char* outfile = *++argv;
	argc -= 2;

	if (argc > 1) {
		int i = 0;
		TOC = 1;
		while (--argc > 0) {
			TOCArray[i++] = atoi(*++argv);
		}
	} else {
		TOC = 0;
	}

	FILE *fp, *fp2;
	int fsize;

	if ((fp = fopen(infile, "rb")) == NULL) {
		printf("Cannot open %s\n", infile);
		exit(1);
	}
	fsize = GetFileSize(infile);

	if ((fp2 = fopen(outfile, "w")) == NULL) {
		printf("Cannot open %s\n", outfile);
		exit(1);
	}

	printf("\n");
	ReadFile(fp, fp2, fsize);

	fclose(fp);
	fclose(fp2);
}

typedef struct _SPeak {
	short	data;
	int	count;
} SPeak;

void ReadFile(FILE* fp, FILE* fp2, int fsize)
{
	int ptr = 0;

	int count = 0;
	int curprogress = 0;
	int progressStep = fsize / 1000;

	int c1, c2, c3, c4;
	short dataL, dataR;
	unsigned short absL, absR;

	SPeak peakL, peakR, vupeakL, vupeakR;
	unsigned short vuShortPeakL, vuShortPeakR;

	unsigned short vuArrayL[PTR_MAX];
	unsigned short vuArrayR[PTR_MAX];
	unsigned long vuSumL, vuSumR;
	unsigned short vuL, vuR;

	int i;
	for (i = 0; i < PTR_MAX; i++) {
		vuArrayL[i] = vuArrayR[i] = 0;
	}
	peakL.data = peakR.data = vupeakL.data = vupeakR.data = 0;
	vuShortPeakL = vuShortPeakR = 0;
	vuSumL = vuSumR = 0;

	while(1) {
		if ((c1 = getc(fp)) == EOF) break;
		if ((c2 = getc(fp)) == EOF) break;
		if ((c3 = getc(fp)) == EOF) break;
		if ((c4 = getc(fp)) == EOF) break;

		dataL = (signed short)((c1 << 8) | c2);
		dataR = (signed short)((c3 << 8) | c4);

		absL = abs(dataL);
		absR = abs(dataR);

		if (peakL.data < absL) {
			peakL.data = absL;
			peakL.count = count;
		}
		if (peakR.data < absR) {
			peakR.data = absR;
			peakR.count = count;
		}

		vuArrayL[ptr] = absL;
		vuArrayR[ptr] = absR;

		{
			int ptrNext = (ptr + 1 < PTR_MAX) ? ptr + 1 : 0;

			vuSumL -= vuArrayL[ptrNext];
			vuSumR -= vuArrayR[ptrNext];
			vuSumL += absL;
			vuSumR += absR;

			ptr = ptrNext;
		}

		vuL = vuSumL / PTR_MAX;
		vuR = vuSumR / PTR_MAX;

		if (vuShortPeakL < vuL) { vuShortPeakL = vuL; }
		if (vuShortPeakR < vuR) { vuShortPeakR = vuR; }
		if (vupeakL.data < vuL) {
			vupeakL.data = vuL;
			vupeakL.count = count;
		}
		if (vupeakR.data < vuR) {
			vupeakR.data = vuR;
			vupeakR.count = count;
		}

		if (count % FS == 0) {
			fprintf(fp2, "%7.3fdB, %7.3fdB\n", rms(vuShortPeakL), rms(vuShortPeakR));
			vuShortPeakL = vuShortPeakR = 0;
		}
		if (count * 4 > curprogress) {
			printf("\x1bM%5.1f%%\n", (double)curprogress / fsize * 100);
			curprogress += progressStep;
		}
		count++;
	}

	printf("\n");
	printf("peakL --> 0x%04x at %s\n", peakL.data, timeformat(peakL.count));
	printf("peakR --> 0x%04x at %s\n", peakR.data, timeformat(peakR.count));

	printf("vuPeakL --> %7.3fdB at %s\n", rms(vupeakL.data), timeformat(vupeakL.count));
	printf("vuPeakR --> %7.3fdB at %s\n", rms(vupeakR.data), timeformat(vupeakR.count));

	fprintf(fp2, "#peakL --> 0x%04x at %s\n", peakL.data, timeformat(peakL.count));
	fprintf(fp2, "#peakR --> 0x%04x at %s\n", peakR.data, timeformat(peakR.count));

	fprintf(fp2, "#vuPeakL --> %7.3fdB at %s\n", rms(vupeakL.data), timeformat(vupeakL.count));
	fprintf(fp2, "#vuPeakR --> %7.3fdB at %s\n", rms(vupeakR.data), timeformat(vupeakR.count));
}
