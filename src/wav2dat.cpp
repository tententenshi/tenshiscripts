#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "wav.h"


void Usage(const char* command)
{
	fprintf(stderr, "print out wav data\n");
	fprintf(stderr, "%s input_wav_file output filename.dat [skip_duty_in_sample] \n\n", command);
}

void Process(FILE *fp, FILE *theWavFile, const SFormatChunk& formatChunk, int dataSize, double startPoint, double endPoint, int skipDutySample)
{
	int wavSamples = dataSize / formatChunk.blockSize;
	int FSAMP = formatChunk.fsamp;
	int num_ch = formatChunk.channel;
	int16_t format = formatChunk.format;

	int aStartSample = (startPoint >= 0) ? (int)(startPoint * FSAMP) : 0;
	int aEndSample = (endPoint >= 0) ? (int)(endPoint * FSAMP) : wavSamples;

	if (fseek(theWavFile, aStartSample * formatChunk.blockSize, SEEK_CUR) != 0) {
		return;
	}

	double aUnityVal = GetUnityVal(&formatChunk);
	for (int i = 0; i < aEndSample - aStartSample; i++) {
		bool aIsSkip = (i % skipDutySample) != 0;
		if (!aIsSkip) {
			fprintf (fp, "%f	", (double)i / FSAMP);
		}
		for (int ch = 0; ch < num_ch; ch++) {
			double val = ReadWaveData(theWavFile, &formatChunk) * aUnityVal;

			if (!aIsSkip) {
				if (format == 1) {
					fprintf (fp, "%d", (int)val);
				} else if (format == 3) {
					fprintf (fp, "%g", val);
				}
				if (ch < num_ch - 1) {
//					fprintf (fp, ", ");
					fprintf (fp, "	");
				}
			}
		}
		if (!aIsSkip) {
			fprintf (fp, "\n");
		}
	}
}

int main(int argc, char* argv[])
{
	if ((argc != 3) && (argc != 4)) {
		Usage(argv[0]);
		exit(1);
	}

//	int wavLen = atoi(*++argv);
	char* inWav = *++argv;
	char* outfile = *++argv;

	double startPoint = -1;
	double endPoint = -1;
	int skipDutySample = 1;
#if 0
	if (argc == 5) {
		char* start = *++argv;
		char* end = *++argv;
		startPoint = atof(start);
		endPoint = atof(end);
	}
#endif
	if (argc == 4) {
		char* skipDutyInSample = *++argv;
		skipDutySample = atoi(skipDutyInSample);
	}

	FILE *fpWav, *fp;

	if ((fpWav = fopen(inWav, "rb")) == NULL) {
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
		Process(fp, fpWav, formatChunkBuf, dataSize, startPoint, endPoint, skipDutySample);
	}

	fclose(fpWav);
	fclose(fp);
}

