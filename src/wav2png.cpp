#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "wav.h"


void Usage(const char* command)
{
	fprintf(stderr, "print out wav as png\n");
//	fprintf(stderr, "%s from_sample to_sample(set -1 for whole sample) output_filename.gif input_wav_file [...]\n\n", command);
	fprintf(stderr, "%s from_sample to_sample(set -1 for whole sample) input_wav_file [...]\n\n", command);
}

void Process(FILE *gp, FILE *theWavFile, const SFormatChunk& formatChunk, int dataSize, double startPoint, double endPoint)
{
	int wavSamples = dataSize / formatChunk.blockSize;
	int num_ch = formatChunk.channel;
	int16_t format = formatChunk.format;

	int aStartSample = (startPoint >= 0) ? (int)(startPoint) : 0;
	int aEndSample = (endPoint >= 0) ? (int)(endPoint) : wavSamples;

	if (fseek(theWavFile, aStartSample * formatChunk.blockSize, SEEK_CUR) != 0) {
		return;
	}

	fprintf(gp, "plot ");
	for (int i = 0; i < num_ch; i++) {
		fprintf(gp, "'-' w lines");
		if (i < num_ch-1) {
			fprintf(gp, ", ");
		}
	}
	fprintf(gp, "\n");

	for (int i = 0; i < aEndSample - aStartSample; i++) {
		for (int ch = 0; ch < num_ch; ch++) {
			double val = ReadWaveData(theWavFile, &formatChunk);

			fprintf(gp, "%g", val);

			if (ch < num_ch - 1) {
				fprintf (gp, ", ");
//				fprintf (gp, "	");
			}
		}
		fprintf(gp, " :%d\n",i);
	}
	fprintf(gp, "e\n");
}

int main(int argc, char* argv[])
{
	if (argc < 4) {
		Usage(argv[0]);
		exit(1);
	}
	argv++;	argc--;

	int From_Sample = atoi(*argv++);	argc--;
	int To_Sample = atoi(*argv++);		argc--;
//	char* outfile = *argv++;			argc--;

	int inWavNum = argc;
	char* inWavArray[inWavNum];
	for (int i = 0; i < inWavNum; i++) {
		inWavArray[i] = *argv++;
	}

	FILE *gp = popen("gnuplot", "w");
//	fprintf(gp, "set terminal gif animate optimize delay 100 size 1024,768\n");
//	fprintf(gp, "set output \"%s\"\n", outfile);
	fprintf(gp, "set terminal pngcairo enhanced size 640, 480\n");
	fprintf(gp, "set yrange[-1:1]\n");
	fprintf(gp, "set nokey\n");

	for (int i = 0; i < inWavNum; i++) {
		FILE *fpWav;
		if ((fpWav = fopen(inWavArray[i], "rb")) == NULL) {
			printf("Cannot open %s\n", inWavArray[i]);
			exit(1);
		}

		SFormatChunk formatChunkBuf;
		int dataSize = ParseWaveHeader(fpWav, &formatChunkBuf);
		int32_t dataStartPos = ftell(fpWav);

		printf("dataSize is %d, output [%d:%d]\n", dataSize, From_Sample, To_Sample);
		if (dataSize > 0) {
			char out_png[512];
			strcpy(out_png, inWavArray[i]);
			strcpy(strstr(out_png, ".wav"), ".png");
			fprintf(gp, "set output \"%s\"\n", out_png);
			Process(gp, fpWav, formatChunkBuf, dataSize, From_Sample, To_Sample);
		}

		fclose(fpWav);
	}

	pclose(gp);
}

