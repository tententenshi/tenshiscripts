#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../filters.h"
#include "../wav.h"


void Usage(const char* command)
{
	fprintf(stderr, "report level on ITU-1770\n");
	fprintf(stderr, "http://web.ck.miyakoda.roland.co.jp/piano/software/dsp/Loudness/Loudness.html\n");
	fprintf(stderr, "%s input_wav_file output filename.wav\n\n", command);
}

void PrepareK_Filter(const SFormatChunk& formatChunk);
void Destruct(const SFormatChunk& formatChunk);
void Process(FILE *fp, FILE *theWavFile, const SFormatChunk& formatChunk, int wavSamples, double theMeasurementInterval_sec);

int main(int argc, char* argv[])
{
	if (argc != 3) {
		Usage(argv[0]);
		exit(1);
	}

//	int wavLen = atoi(*++argv);
	char* inWav = *++argv;
	char* outfile = *++argv;

	FILE *fpWav, *fpOut;

	if ((fpWav = fopen(inWav, "rb")) == NULL) {
		printf("Cannot open %s\n", inWav);
		exit(1);
	}
	if ((fpOut = fopen(outfile, "wb")) == NULL) {
		printf("Cannot open %s\n", outfile);
		exit(1);
	}

	SFormatChunk formatChunkIn;
	SFormatChunk formatChunkOut;
	int32_t dataSize = ParseWaveHeader(fpWav, &formatChunkIn);
	int32_t wavSamples = dataSize / formatChunkIn.blockSize;
	int32_t dataStartPos;

	if (dataSize > 0) {
		PrepareK_Filter(formatChunkIn);

		dataStartPos = CreateWavHeader(fpOut, &formatChunkOut, 1, formatChunkIn.fsamp, formatChunkIn.bitLength, formatChunkIn.format);

		Process(fpOut, fpWav, formatChunkIn, wavSamples, 0.4/*sec*/);
		int32_t dataSizeOut = wavSamples * formatChunkOut.blockSize;
		MaintainWavHeader(fpOut, dataSizeOut, dataStartPos);

		Destruct(formatChunkIn);
	}

	fclose(fpWav);
	fclose(fpOut);
}

#include "../list.h"
static CList<CFilterBase*>* spList;
void PrepareK_Filter(const SFormatChunk& formatChunk)
{
	int FSAMP = formatChunk.fsamp;
	int num_ch = formatChunk.channel;
	spList = new CList<CFilterBase*> [num_ch];

	for (int ch = 0; ch < num_ch; ch++) {
		{
			CFilterBase* aIns = new CHPF2(FSAMP);      spList[ch].push_back(aIns);
			((CHPF2*)aIns)->SetProperty(38.135470876113, 0.500327037325042);
		}
		{
			CFilterBase* aIns = new CHSV2(FSAMP);    spList[ch].push_back(aIns);
			((CHSV2*)aIns)->SetProperty(1681.97445095553, 3.99984385397335, 0.707175236955419);
		}
	}

	for (CList<CFilterBase*>::Iterator iter = spList[0].begin(); iter != spList[0].end(); iter++) {
		if (iter != 0) {
			(*iter)->ShowProperty();
			(*iter)->ShowCoef();
		}
	}
}

void Destruct(const SFormatChunk& formatChunk)
{
	int num_ch = formatChunk.channel;
	for (int ch = 0; ch < num_ch; ch++) {
		for (CList<CFilterBase*>::Iterator iter = spList[ch].begin(); iter != spList[ch].end(); iter++) {
			if (iter != 0) {
				delete *iter;
			}
		}
	}
	delete [] spList;
}

void Process(FILE *fp, FILE *theWavFile, const SFormatChunk& formatChunk, int wavSamples, double theMeasurementInterval_sec)
{
	int FSAMP = formatChunk.fsamp;
	int num_ch = formatChunk.channel;
	int MeasurementInterval_sample = (int)(theMeasurementInterval_sec * FSAMP + 0.5);

	double** buf = new double* [num_ch];
	for (int ch = 0; ch < num_ch; ch++) {
		buf[ch] = new double [MeasurementInterval_sample];
		memset(buf[ch], 0, sizeof(double) * MeasurementInterval_sample);
	}
	double* sum = new double [num_ch];
	for (int ch = 0; ch < num_ch; ch++) {
		sum[ch] = 0;
	}

	int aRingPointer = 0;
	for (int i = 0; i < wavSamples; i++) {
		int aNewRingPointer = (aRingPointer + 1) % MeasurementInterval_sample;
		double aChannel_sum = 0;
		for (int ch = 0; ch < num_ch; ch++) {
			double val = ReadWaveData(theWavFile, &formatChunk);

			for (CList<CFilterBase*>::Iterator iter = spList[ch].begin(); iter != spList[ch].end(); iter++) {
				if (iter != 0) {
					val = (*iter)->Push(val);
				}
			}
			double mean_square = val * val / MeasurementInterval_sample;
			sum[ch] = fmax(sum[ch] + mean_square - buf[ch][aNewRingPointer], 0);
			buf[ch][aRingPointer] = mean_square;

			aChannel_sum += sum[ch];
		}
		WriteWaveData(fp, &formatChunk, (i < MeasurementInterval_sample) ? 0 : sqrt(aChannel_sum) * 0.9235/*-0.691dB*/);
		aRingPointer = aNewRingPointer;
	}

	for (int ch = 0; ch < num_ch; ch++) {
		delete [] buf[ch];
	}
	delete [] buf;
	delete [] sum;
}
