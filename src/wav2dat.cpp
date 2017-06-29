#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "wav.h"


void Usage(const char* command)
{
	fprintf(stderr, "print out wav data\n");
	fprintf(stderr, "%s input_wav_file output filename.dat\n\n", command);
}

void Process(FILE *fp, FILE *theWavFile, const SFormatChunk& formatChunk, int dataSize)
{
	int wavSamples = dataSize / formatChunk.blockSize;
	int FSAMP = formatChunk.fsamp;
	int num_ch = formatChunk.channel;
	int16_t format = formatChunk.format;

	for (int i = 0; i < wavSamples; i++) {
		for (int ch = 0; ch < num_ch; ch++) {
			double val = ReadWaveData(theWavFile, &formatChunk);
			if (format == 1) {
				fprintf (fp, "%d  ", val);
			} else if (format == 3) {
				fprintf (fp, "%g  ", val);
			}
		}
		fprintf (fp, "\n");
	}
}

int main(int argc, char* argv[])
{
	if (argc != 3) {
		Usage(argv[0]);
		exit(1);
	}

//	int wavLen = atoi(*++argv);
	char* inWav = *++argv;
	char* outfile = *++argv;

	FILE *fpWav, *fp;

	if ((fpWav = fopen(inWav, "r")) == NULL) {
		printf("Cannot open %s\n", inWav);
		exit(1);
	}
	if ((fp = fopen(outfile, "w")) == NULL) {
		printf("Cannot open %s\n", outfile);
		exit(1);
	}

	SFormatChunk formatChunkBuf;
	int dataSize = ParseWaveHeader(fpWav, &formatChunkBuf);
	int32_t dataStartPos = ftell(fpWav);

	printf("dataSize is %d\n", dataSize);
	if (dataSize > 0) {
		Process(fp, fpWav, formatChunkBuf, dataSize);
	}

	fclose(fpWav);
	fclose(fp);
}

