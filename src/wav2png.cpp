#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "wav.h"


void Usage(const char* command)
{
	fprintf(stderr, "print out wav as png\n");
//	fprintf(stderr, "%s from_sample to_sample(set -1 for whole sample) output_filename.gif input_wav_file [...]\n\n", command);
	fprintf(stderr, "%s from_sample to_sample(set -1 for whole sample) channel(set -1 for whole channel) input_wav_file [...]\n\n", command);
}

void Process(FILE *gp, FILE *theWavFile, const SFormatChunk& formatChunk, int dataStartPos, int dataSize, double startPoint, double endPoint, int Channel)
{
	int wavSamples = dataSize / formatChunk.blockSize;
	int num_ch = formatChunk.channel;
	int16_t format = formatChunk.format;

	int aStartSample = (startPoint >= 0) ? (int)(startPoint) : 0;
	int aEndSample = (endPoint >= 0) ? (int)(endPoint) : wavSamples;

	if (aStartSample > aEndSample) return;
	if (fseek(theWavFile, dataStartPos + aStartSample * formatChunk.blockSize, SEEK_SET) != 0) {
		return;
	}

	double** aDataBuf = new double* [num_ch];
	for (int i = 0; i < num_ch; i++) {
		aDataBuf[i] = new double [aEndSample - aStartSample + 1];
	}
	for (int i = 0; i <= aEndSample - aStartSample; i++) {
		for (int ch = 0; ch < num_ch; ch++) {
			aDataBuf[ch][i] = ReadWaveData(theWavFile, &formatChunk);
		}
	}

	fprintf(gp, "plot ");
	if (Channel == -1) {
		for (int i = 0; i < num_ch; i++) {
			fprintf(gp, "'-' w lines");
			if (i < num_ch-1) {
				fprintf(gp, ", ");
			}
		}
	} else {
		fprintf(gp, "'-' w lines");
	}
	fprintf(gp, "\n");

	if (Channel == -1) {
		for (int ch = 0; ch < num_ch; ch++) {
			for (int i = 0; i <= aEndSample - aStartSample; i++) {
				fprintf(gp, "%d ",i);
				fprintf(gp, "%g", aDataBuf[ch][i]);

				if (ch < num_ch - 1) {
//					fprintf (gp, ", ");
					fprintf (gp, "	");
				}
				fprintf(gp, "\n");
			}
			fprintf(gp, "e\n");
		}
	} else {
		for (int i = 0; i <= aEndSample - aStartSample; i++) {
			fprintf(gp, "%d ",i);
			fprintf(gp, "%g", aDataBuf[Channel][i]);
			fprintf(gp, "\n");
		}
		fprintf(gp, "e\n");
	}

	for (int i = 0; i < num_ch; i++) {
		delete [] aDataBuf[i];
	}
	delete [] aDataBuf;
}

int main(int argc, char* argv[])
{
	if (argc < 5) {
		Usage(argv[0]);
		exit(1);
	}
	argv++;	argc--;

	int From_Sample = atoi(*argv++);	argc--;
	int To_Sample = atoi(*argv++);		argc--;
//	char* outfile = *argv++;			argc--;
	int Channel = atoi(*argv++);	argc--;

	int inWavNum = argc;
	char* inWavArray[inWavNum];
	for (int i = 0; i < inWavNum; i++) {
		inWavArray[i] = *argv++;
	}

#if 1
	FILE *gp = popen("gnuplot", "w");
#else
	FILE *gp;
	if ((gp = fopen("wav2png_debug.txt", "w")) == NULL) {
		printf("Cannot open %s\n", "wav2png_debug.txt");
		exit(1);
	}
#endif
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
			char out_basename[512];
			strcpy(out_basename, inWavArray[i]);
			*strstr(out_basename, ".wav") = '\0';

			char out_png[512];
			sprintf(out_png, "%s.png", out_basename);
			FILE *fpOutTest;
			if ((fpOutTest = fopen(out_png, "r")) == NULL) {
			} else {
				fclose(fpOutTest);
				for (int i = 0; i < 1000; i++) {
					sprintf(out_png, "%s_%03d.png", out_basename, i);
					if ((fpOutTest = fopen(out_png, "r")) == NULL) {
						break;
					} else {
						fclose(fpOutTest);
					}
				}
			}

			fprintf(gp, "set output \"%s\"\n", out_png);
			Process(gp, fpWav, formatChunkBuf, dataStartPos, dataSize, From_Sample, To_Sample, Channel);
//			Process(stderr, fpWav, formatChunkBuf, dataStartPos, dataSize, From_Sample, To_Sample);
		}

		fclose(fpWav);
	}

	pclose(gp);
}

