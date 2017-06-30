#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../filters.h"
#include "../wav.h"


void Usage(const char* command)
{
	fprintf(stderr, "make wave through specified filters\n");
	fprintf(stderr, "%s input_wav_file modulator_conf.txt output filename.wav\n\n", command);
	fprintf(stderr, "<Format of modulator_conf.txt File>\n");
	fprintf(stderr, "type freq initial_phase_as_degree\n");
	fprintf(stderr, "type freq initial_phase_as_degree\n");
	fprintf(stderr, "           :\n\n");
	fprintf(stderr, "\tkind of types\n");
	fprintf(stderr, "\t\tATT       -- Input Gain\n");
	fprintf(stderr, "\t\tDELAY     -- delay[sec]\n");
	fprintf(stderr, "\t\tSINE      -- sine modulator\n");
}

static void ParseInputFile(FILE *fp, const SFormatChunk& formatChunk);
static void Destruct(const SFormatChunk& formatChunk);
static void Process(FILE *fp, FILE *theWavFile, const SFormatChunk& formatChunk, int dataSize);
static void CopyFile(FILE* fpSrc, FILE* fpDst, int32_t copySize);

int main(int argc, char* argv[])
{
	if (argc != 4) {
		Usage(argv[0]);
		exit(1);
	}

//	int wavLen = atoi(*++argv);
	char* inWav = *++argv;
	char* infile = *++argv;
	char* outfile = *++argv;

	FILE *fpWav, *fp, *fp2;

	if ((fpWav = fopen(inWav, "r")) == NULL) {
		printf("Cannot open %s\n", inWav);
		exit(1);
	}
	if ((fp = fopen(infile, "r")) == NULL) {
		printf("Cannot open %s\n", infile);
		exit(1);
	}
	if ((fp2 = fopen(outfile, "w")) == NULL) {
		printf("Cannot open %s\n", outfile);
		exit(1);
	}

	SFormatChunk formatChunkBuf;
	int32_t dataSize = ParseWaveHeader(fpWav, &formatChunkBuf);
	int32_t dataStartPos = ftell(fpWav);
	rewind(fpWav);

	if (dataSize > 0) {
		ParseInputFile(fp, formatChunkBuf);
		CopyFile(fpWav, fp2, dataStartPos);
		Process(fp2, fpWav, formatChunkBuf, dataSize);

		Destruct(formatChunkBuf);

		CopyFile(fpWav, fp2, -1);
	}

	fclose(fpWav);
	fclose(fp);
	fclose(fp2);
}

enum EType {
	ATT,
	DELAY,
	SINE,
	NONE,
};

EType ParseEType(const char* str)
{
	if (strcmp(str, "ATT") == 0) {
		return ATT;
	} else if (strcmp(str, "DELAY") == 0) {
		return DELAY;
	} else if (strcmp(str, "SINE") == 0) {
		return SINE;
	}
	return NONE;
}

#include "../list.h"
static CList<CFilterBase*>* spList;
void ParseInputFile(FILE *fp, const SFormatChunk& formatChunk)
{
	int FSAMP = formatChunk.fsamp;
	int num_ch = formatChunk.channel;
	spList = new CList<CFilterBase*> [num_ch];

	for (int ch = 0; ch < num_ch; ch++) {
		rewind(fp);

		while (1) {
			char aBuf[101];
			if (fgets(aBuf, 100, fp) == 0) { break; }

#if 0
			aRetVal = fscanf(fp, "%10s %f %f %f\n", aTypeStr, freq, gain, q);

			printf("%s, %f, %f, %f\n", aTypeStr, freq, gain, q);
#endif
			int aColNum;
			char aTypeStr[31];
			double freq, initial_phase;
			aColNum = sscanf(aBuf, "%30s %lf %lf", aTypeStr, &freq, &initial_phase);
//			printf("%s, %f, %f %d\n", aTypeStr, freq, initial_phase, aColNum);
			if (aColNum < 2) { continue; }	// skip empty line

			EType aType = ParseEType(aTypeStr);
			switch (aType) {
			case ATT:
				{ CFilterBase* aIns = new CATT(FSAMP);       spList[ch].push_back(aIns);
					if (aColNum == 2) {
						((CATT*)aIns)->SetProperty(freq);
					} else if (aColNum >= 3) {
						((CATT*)aIns)->SetProperty(initial_phase);
					}
				}
				break;
			case DELAY:
				{ CFilterBase* aIns = new CDELAY(FSAMP);     spList[ch].push_back(aIns); ((CDELAY*)aIns)->SetProperty(freq); }
				break;
			case SINE:
				{ CFilterBase* aIns = new CSINE(FSAMP);      spList[ch].push_back(aIns); ((CSINE*)aIns)->SetProperty(freq, initial_phase); }
				break;
			default:
				printf("skipping unknown type %s -- %s", aTypeStr, aBuf);
				break;
			}
		}
//		spList[ch].push_back(theMgr);
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

void Process(FILE *fp, FILE *theWavFile, const SFormatChunk& formatChunk, int dataSize)
{
	int wavSamples = dataSize / formatChunk.blockSize;
	int FSAMP = formatChunk.fsamp;
	int num_ch = formatChunk.channel;

	for (int i = 0; i < wavSamples; i++) {
		for (int ch = 0; ch < num_ch; ch++) {
			double val = ReadWaveData(theWavFile, &formatChunk);
		
			for (CList<CFilterBase*>::Iterator iter = spList[ch].begin(); iter != spList[ch].end(); iter++) {
				if (iter != 0) {
					val = (*iter)->Push(val);
				}
			}

			WriteWaveData(fp, &formatChunk, val);
		}
	}
}

// --------------------------------------------------------------------
enum { BUF_SIZE = 0x100000, };
static unsigned char buf[BUF_SIZE];

static void CopyFile(FILE* fpSrc, FILE* fpDst, int32_t theSize)
{
	size_t remainSize = theSize;
	size_t copySize = BUF_SIZE;
	if ((remainSize >= 0) && (remainSize <= copySize)) { copySize = remainSize; }
	while (copySize > 0) {
		size_t readSize = fread(buf, 1, copySize, fpSrc);
		if (readSize <= 0) { break; }
		size_t writeSize = fwrite(buf, 1, readSize, fpDst);
		if (writeSize <= 0) { break; }
		if (remainSize >= 0) {
			remainSize -= readSize;
			if (remainSize <= copySize) { copySize = remainSize; }
		}
	}
}

// --------------------------------------------------------------------
