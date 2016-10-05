#ifndef _wav_h_
#define _wav_h_

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

typedef struct
{
	char           name[4];
	long           chunkSize;
	short          format;
	unsigned short channel;
	unsigned long  fsamp;
	unsigned long  dataRate;
	unsigned short blockSize;
	unsigned short bitLength;
	/* Note: there may be additional fields here, depending upon wFormatTag. */ 
} SFormatChunk;

typedef struct
{
	char           name[4];
	long           chunkSize;
} SDataChunk;

enum WAV_FORMAT_TYPE {
	unknown		= 0x0000,
	PCM			= 0x0001,
	FloatPCM	= 0x0003,
};

static int ParseWaveHeader(FILE* fp, SFormatChunk* pFormatChunkBuf)
{
	rewind(fp);
	char buf[5];
	buf[4] = '\0';
	fread(buf, 4, 1, fp);
	if (strncmp(buf, "RIFF", 4) != 0) {
		printf("RIFF-->%s\n", buf);
		return -1;
	}
	fread(buf, 4, 1, fp);	// total size
	fread(buf, 4, 1, fp);
	if (strncmp(buf, "WAVE", 4) != 0) {
		printf("WAVE-->%s\n", buf);
		return -1;
	}

	fread(pFormatChunkBuf, sizeof(SFormatChunk), 1, fp);
	if (strncmp(pFormatChunkBuf->name, "fmt ", 4) != 0) {
		printf("fmt -->%s\n", pFormatChunkBuf->name);
		return -1;
	}

	fseek(fp, pFormatChunkBuf->chunkSize + 8 - sizeof(SFormatChunk), SEEK_CUR);

	SDataChunk dataChunkBuf;
	fread(&dataChunkBuf, sizeof(SDataChunk), 1, fp);
	while (strncmp(dataChunkBuf.name, "data", 4) != 0) {
		printf("skipping %s chunk\n", dataChunkBuf.name);

		if (fseek(fp, dataChunkBuf.chunkSize, SEEK_CUR) != 0) {
			printf("data chunk not found\n");
			return -1;
		}
		fread(&dataChunkBuf, sizeof(SDataChunk), 1, fp);
	}

	printf("data size = %d\n", dataChunkBuf.chunkSize);

	return dataChunkBuf.chunkSize;
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
			signed short sval;
			fread(&sval, 2, 1, fpWav);
			return (double)sval;
		} else if (bitLength == 24) {
			signed long lval;
			fread(&lval, 3, 1, fpWav);
			lval <<= 8;	// 24 --> 32
			return (double)lval;
		} else if (bitLength == 32) {
			signed long lval;
			fread(&lval, 4, 1, fpWav);
			return (double)lval;
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
	} else {
		return 0;
	}
}

static void WriteWaveData(FILE* fp, const SFormatChunk* formatChunk, double val)
{
	int format = formatChunk->format;
	int bitLength = formatChunk->bitLength;
	if (format == 1) {
		if (bitLength == 16) {
			val = (val > 0x7fff) ? 0x7fff : ((val < -0x8000) ? -0x8000 : val);
			signed short aData = (signed short)val;
			fwrite(&aData, 2, 1, fp);
		} else if (bitLength == 24) {
			val = (val > 0x7fffff00) ? 0x7fffff00 : ((val < -(double)0x80000000) ? -(double)0x80000000 : val);
			signed long aData = (signed long)val;
			aData >>= 8;
			fwrite(&aData, 3, 1, fp);
		} else if (bitLength == 32) {
			val = (val > 0x7fffffff) ? 0x7fffffff : ((val < -(double)0x80000000) ? -(double)0x80000000 : val);
			signed long aData = (signed long)val;
			fwrite(&aData, 4, 1, fp);
		}
	} else if (format == 3) {
		if (bitLength == 32) {
			float fval = val;
			fwrite(&fval, 4, 1, fp);
		} else if (bitLength == 64) {
			fwrite(&val, 8, 1, fp);
		}
	} else {
		return;
	}
}

void PutLongVal(unsigned long val_l, FILE* fp)
{
	int i;
	for (i = 0; i < 4; i++, (val_l >>= 8)) {
		fputc(val_l & 0xff, fp);
	}
}
void PutShortVal(unsigned short val_s, FILE* fp)
{
	int i;
	for (i = 0; i < 2; i++, (val_s >>= 8)) {
		fputc(val_s & 0xff, fp);
	}
}

long CreateWavHeader(FILE* fp, const SFormatChunk* formatChunk)
{
	int num_ch = formatChunk->channel;
	int FSAMP = formatChunk->fsamp;
	int bitLength = formatChunk->bitLength;
	int format = formatChunk->format;

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

void MaintainWavHeader(FILE* fp, const SFormatChunk* formatChunk, int dataSize, long dataStartPos)
{
	struct stat stbuf;
	int file_size;
	if (fstat(fileno(fp), &stbuf) == -1) {
		file_size = 0;
	} else {
		file_size = stbuf.st_size;
	}

	fseek(fp, 4, SEEK_SET);
	PutLongVal(file_size, fp);	// total size

	fseek(fp, dataStartPos - 4, SEEK_SET);
	PutLongVal(dataSize, fp);
}

#endif // _wav_h_
