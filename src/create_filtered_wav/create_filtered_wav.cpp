#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../filters.h"
#include "../wav.h"


void Usage(const char* command)
{
	fprintf(stderr, "make wave through specified filters\n");
	fprintf(stderr, "%s input_wav_file <input filename.txt> <output filename.wav>\n\n", command);
	fprintf(stderr, "<Format of Input txt File>\n");
	fprintf(stderr, "type freq gain q\n");
	fprintf(stderr, "type freq gain q\n");
	fprintf(stderr, "         :\n\n");
	fprintf(stderr, "\tkind of types\n");
	fprintf(stderr, "\t\tATT       -- Input Gain\n");
	fprintf(stderr, "\t\tDELAY     -- Delay[sec]\n");
	fprintf(stderr, "\t\tHPF1      -- 1st order high pass filter\n");
	fprintf(stderr, "\t\tLPF1      -- 1st order low pass filter\n");
	fprintf(stderr, "\t\tHSV1_Trad -- 1st order high shelving filter (traditional operation)\n");
	fprintf(stderr, "\t\tLSV1_Trad -- 1st order low shelving filter (traditional operation)\n");
	fprintf(stderr, "\t\tHSV1      -- 1st order high shelving filter\n");
	fprintf(stderr, "\t\tLSV1      -- 1st order low shelving filter\n");
	fprintf(stderr, "\t\tHPF2      -- 2nd order high pass filter\n");
	fprintf(stderr, "\t\tLPF2      -- 2nd order low pass filter\n");
	fprintf(stderr, "\t\tHSV2      -- 2nd order high shelving filter\n");
	fprintf(stderr, "\t\tLSV2      -- 2nd order low shelving filter\n");
	fprintf(stderr, "\t\tPKG2      -- 2nd order peaking filter\n");
}

void ParseInputFile(FILE *fp, const SFormatChunk& formatChunk);
void Destruct(const SFormatChunk& formatChunk);
//void SetupWavHead(FILE* fp, const SFormatChunk& formatChunk, int dataSize);
void Process(FILE *fp, FILE *theWavFile, const SFormatChunk& formatChunk, int dataSize);
static void CopyFile(FILE* fpSrc, FILE* fpDst, long copySize);

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
	long dataSize = ParseWaveHeader(fpWav, &formatChunkBuf);
	long dataStartPos = ftell(fpWav);
	rewind(fpWav);

	if (dataSize > 0) {
		ParseInputFile(fp, formatChunkBuf);

		CopyFile(fpWav, fp2, dataStartPos);
//		SetupWavHead(fp2, formatChunkBuf, dataSize);
		Process(fp2, fpWav, formatChunkBuf, dataSize);
		CopyFile(fpWav, fp2, -1);

		Destruct(formatChunkBuf);
	}

	fclose(fpWav);
	fclose(fp);
	fclose(fp2);
}

enum EType {
	ATT,
	DELAY,
	HPF1,
	LPF1,
	HSV1_Trad,
	LSV1_Trad,
	HSV1,
	LSV1,
	HPF2,
	LPF2,
	HSV2,
	LSV2,
	PKG2,
	NONE,
};

EType ParseEType(const char* str)
{
	if (strcmp(str, "ATT") == 0) {
		return ATT;
	} else if (strcmp(str, "DELAY") == 0) {
		return DELAY;
	} else if (strcmp(str, "HPF1") == 0) {
		return HPF1;
	} else if (strcmp(str, "LPF1") == 0) {
		return LPF1;
	} else if (strcmp(str, "HSV1_Trad") == 0) {
		return HSV1_Trad;
	} else if (strcmp(str, "LSV1_Trad") == 0) {
		return LSV1_Trad;
	} else if (strcmp(str, "HSV1") == 0) {
		return HSV1;
	} else if (strcmp(str, "LSV1") == 0) {
		return LSV1;
	} else if (strcmp(str, "HPF2") == 0) {
		return HPF2;
	} else if (strcmp(str, "LPF2") == 0) {
		return LPF2;
	} else if (strcmp(str, "HSV2") == 0) {
		return HSV2;
	} else if (strcmp(str, "LSV2") == 0) {
		return LSV2;
	} else if (strcmp(str, "PKG2") == 0) {
		return PKG2;
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
			double freq, gain, q;
			aColNum = sscanf(aBuf, "%30s %lf %lf %lf", aTypeStr, &freq, &gain, &q);
//			printf("%s, %f, %f, %f %d\n", aTypeStr, freq, gain, q, aColNum);
			if (aColNum < 2) { continue; }	// skip empty line

			EType aType = ParseEType(aTypeStr);
			switch (aType) {
			case ATT:
				{ CFilterBase* aIns = new CATT(FSAMP);       spList[ch].push_back(aIns);
					if (aColNum == 2) {
						((CATT*)aIns)->SetProperty(freq);
					} else if (aColNum >= 3) {
						((CATT*)aIns)->SetProperty(gain);
					}
				}
				break;
			case DELAY:
				{ CFilterBase* aIns = new CDELAY(FSAMP);     spList[ch].push_back(aIns); ((CDELAY*)aIns)->SetProperty(freq); }
				break;
			case HPF1:
				{ CFilterBase* aIns = new CHPF1(FSAMP);      spList[ch].push_back(aIns); ((CHPF1*)aIns)->SetProperty(freq); }
				break;
			case LPF1:
				{ CFilterBase* aIns = new CLPF1(FSAMP);      spList[ch].push_back(aIns); ((CLPF1*)aIns)->SetProperty(freq); }
				break;
			case HSV1_Trad:
				{ CFilterBase* aIns = new CHSV1_Trad(FSAMP); spList[ch].push_back(aIns); ((CHSV1_Trad*)aIns)->SetProperty(freq, gain); }
				break;
			case LSV1_Trad:
				{ CFilterBase* aIns = new CLSV1_Trad(FSAMP); spList[ch].push_back(aIns); ((CLSV1_Trad*)aIns)->SetProperty(freq, gain); }
				break;
			case HSV1:
				{ CFilterBase* aIns = new CHSV1(FSAMP);    spList[ch].push_back(aIns); ((CHSV1*)aIns)->SetProperty(freq, gain); }
				break;
			case LSV1:
				{ CFilterBase* aIns = new CLSV1(FSAMP);    spList[ch].push_back(aIns); ((CLSV1*)aIns)->SetProperty(freq, gain); }
				break;
			case HPF2:
				{ CFilterBase* aIns = new CHPF2(FSAMP);      spList[ch].push_back(aIns);
					if (aColNum == 4) {
						((CHPF2*)aIns)->SetProperty(freq, q);
					} else if (aColNum == 3) {
						((CHPF2*)aIns)->SetProperty(freq, gain);
					}
				}
				break;
			case LPF2:
				{ CFilterBase* aIns = new CLPF2(FSAMP);      spList[ch].push_back(aIns);
					if (aColNum == 4) {
						((CLPF2*)aIns)->SetProperty(freq, q);
					} else {
						((CLPF2*)aIns)->SetProperty(freq, gain);
					}
				}
				break;
			case HSV2:
				{ CFilterBase* aIns = new CHSV2(FSAMP);    spList[ch].push_back(aIns); ((CHSV2*)aIns)->SetProperty(freq, gain, q); }
				break;
			case LSV2:
				{ CFilterBase* aIns = new CLSV2(FSAMP);    spList[ch].push_back(aIns); ((CLSV2*)aIns)->SetProperty(freq, gain, q); }
				break;
			case PKG2:
				{ CFilterBase* aIns = new CPKG2(FSAMP);    spList[ch].push_back(aIns); ((CPKG2*)aIns)->SetProperty(freq, gain, q); }
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

enum { BUF_SIZE = 0x100000, };
static unsigned char buf[BUF_SIZE];

static void CopyFile(FILE* fpSrc, FILE* fpDst, long theSize)
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

