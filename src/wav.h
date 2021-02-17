#ifndef _wav_h_
#define _wav_h_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>


typedef struct {
	int16_t 	format;
	uint16_t	channel;
	uint32_t	fsamp;
	uint32_t	dataRate;
	uint16_t	blockSize;
	uint16_t	bitLength;
	/* Note: there may be additional fields here, depending upon wFormatTag. */ 
} SFormatChunk;


enum WAV_FORMAT_TYPE {
	unknown		= 0x0000,
	PCM			= 0x0001,
	FloatPCM	= 0x0003,
};

static int ParseWaveHeader(FILE* fp, SFormatChunk* pFormatChunkBuf)
{
	rewind(fp);
	char buf1[5];
	char buf2[5];
	buf1[4] = '\0';

	fread(buf1, 4, 1, fp);
	if (strncmp(buf1, "RIFF", 4) != 0) {
		printf("RIFF-->%s\n", buf1);
		return -1;
	}
	fread(buf2, 4, 1, fp);	// total size
	fread(buf1, 4, 1, fp);
	if (strncmp(buf1, "WAVE", 4) != 0) {
		printf("WAVE-->%s\n", buf1);
		return -1;
	}

	uint32_t aDataSize = 0;
	while (1) {
		fread(buf1, 4, 1, fp);
		fread(buf2, 4, 1, fp);
		uint32_t aChunkSize = *(uint32_t*)buf2;
		if (strncmp(buf1, "fmt ", 4) == 0) {
			// format chunk found
			fread(pFormatChunkBuf, sizeof(SFormatChunk), 1, fp);
			fseek(fp, aChunkSize - sizeof(SFormatChunk), SEEK_CUR);
		} else if (strncmp(buf1, "data", 4) == 0) {
			aDataSize = aChunkSize;
			break;
		} else {
			printf("skipping %s chunk\n", buf1);
			if (fseek(fp, aChunkSize, SEEK_CUR) != 0) {
				printf("data chunk not found\n");
				return -1;
			}
		}
	}

	printf("data size = %d\n", aDataSize);

	return aDataSize;
}

static double GetUnityVal(const SFormatChunk* formatChunk)
{
	int format = formatChunk->format;
	int bitLength = formatChunk->bitLength;
	if (format == 1) {
		if (bitLength == 16) {
			return 0x1.0p15;
		} else if (bitLength == 24) {
			return 0x1.0p31;
		} else if (bitLength == 32) {
			return 0x1.0p31;
		}
	} else if (format == 3) {
		if (bitLength == 32) {
			return 1.0;
		} else if (bitLength == 64) {
			return 1.0;
		}
	} else {
		return 0;
	}
}

static double ReadWaveData(FILE* fpWav, const SFormatChunk* formatChunk)
{
	int format = formatChunk->format;
	int bitLength = formatChunk->bitLength;
	if (format == 1) {
		if (bitLength == 16) {
			int16_t sval;
			fread(&sval, 2, 1, fpWav);
			return (double)sval / 0x8000;
		} else if (bitLength == 24) {
			int32_t lval;
			fread(&lval, 3, 1, fpWav);
			lval <<= 8;	// 24 --> 32
			return (double)lval / 0x8000 / 0x10000;
		} else if (bitLength == 32) {
			int32_t lval;
			fread(&lval, 4, 1, fpWav);
			return (double)lval / 0x8000 / 0x10000;
		}
	} else if (format == 3) {
		if (bitLength == 32) {
			float fval;
			fread(&fval, 4, 1, fpWav);
			return (double)fval;
		} else if (bitLength == 64) {
			double dval;
			fread(&dval, 8, 1, fpWav);
			return dval;
		}
	}
	return 0;
}

static inline double Bound(double min, double val, double max)
{
	return (val < min) ? min : (val > max) ? max : val;
}

static void WriteWaveData(FILE* fp, const SFormatChunk* formatChunk, double val)
{
	int format = formatChunk->format;
	int bitLength = formatChunk->bitLength;
	if (format == 1) {
		if (bitLength == 16) {
			val = Bound(-0x8000, val * 0x8000, 0x7fff);
			int16_t aData = (int16_t)val;
			fwrite(&aData, 2, 1, fp);
		} else if (bitLength == 24) {
			val = Bound(-(double)0x80000000, val * 0x8000 * 0x10000, 0x7fffffff);
			int32_t aData = (int32_t)val;
			aData >>= 8;
			fwrite(&aData, 3, 1, fp);
		} else if (bitLength == 32) {
			val = Bound(-(double)0x80000000, val * 0x8000 * 0x10000, 0x7fffffff);
			int32_t aData = (int32_t)val;
			fwrite(&aData, 4, 1, fp);
		}
	} else if (format == 3) {
		if (bitLength == 32) {
			float fval = val;
			fwrite(&fval, 4, 1, fp);
		} else if (bitLength == 64) {
			fwrite(&val, 8, 1, fp);
		}
	}
}

void PutLongVal(uint32_t val_l, FILE* fp)
{
	int i;
	for (i = 0; i < 4; i++, (val_l >>= 8)) {
		fputc(val_l & 0xff, fp);
	}
}
void PutShortVal(uint16_t val_s, FILE* fp)
{
	int i;
	for (i = 0; i < 2; i++, (val_s >>= 8)) {
		fputc(val_s & 0xff, fp);
	}
}

int32_t CreateWavHeader(FILE* fp, SFormatChunk* formatChunk, int num_ch, int FSAMP, int bitLength, int format)
{
	formatChunk->format = format;
	formatChunk->channel = num_ch;
	formatChunk->fsamp = FSAMP;
	formatChunk->bitLength = bitLength;
	formatChunk->dataRate = FSAMP * num_ch * bitLength / 8;
	formatChunk->blockSize = bitLength / 8 * num_ch;

	fputs("RIFF", fp);
	PutLongVal(0, fp);	// total size
	fputs("WAVE", fp);
	fputs("fmt ", fp);
	PutLongVal(16, fp);	// size of fmt chunk
	PutShortVal(format, fp);	// format Linear PCM
	PutShortVal(num_ch, fp);	// num of channel
	PutLongVal(FSAMP, fp);	// FSAMP
	PutLongVal(FSAMP * num_ch * bitLength / 8, fp);	// Data Rate
	PutShortVal(bitLength / 8 * num_ch, fp);	// Block Size
	PutShortVal(bitLength, fp);	// Bit Length

	fputs("data", fp);
	PutLongVal(0, fp);	// data size

	return ftell(fp);	// return data start position
}

void MaintainWavHeader(FILE* fp, int dataSize, int32_t dataStartPos)
{
	fflush(fp);

#if 0
	struct stat stbuf;
	int file_size;
	if (fstat(_fileno(fp), &stbuf) == -1) {
		file_size = 0;
	} else {
		file_size = stbuf.st_size;
	}
#else
	fseek(fp, 0L, SEEK_END);
	int file_size = ftell(fp);
#endif

	fseek(fp, 4, SEEK_SET);
	PutLongVal(file_size, fp);	// total size

	fseek(fp, dataStartPos - 4, SEEK_SET);
	PutLongVal(dataSize, fp);
}

enum eEndian {
	eUNKNOWN, eBIG, eLITTLE,
};
eEndian ConfirmSystemEndian(void)
{
	union UTest {
		unsigned long longVal;
		unsigned char charVal[4];
	};
	UTest testVal;
	testVal.longVal = 0x12345678;

	if (testVal.charVal[0] = 0x12) {
		return eBIG;
	} else if (testVal.charVal[0] = 0x12) {
		return eLITTLE;
	} else {
		return eUNKNOWN;
	}
}

#endif // _wav_h_
