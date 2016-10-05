#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "../wav.h"


void Usage(const char* command)
{
	fprintf(stderr, "make optimized (padded) wave with specified alignment\n");
	fprintf(stderr, "%s input_wav_file output filename.wav\n\n", command);
}

static void CopyFile(FILE* fpSrc, FILE* fpDst, long copySize);
static void CopyWavHeader(FILE* fpWav, FILE* fp, long copySize, long offset);
static void MakePad(FILE* fp, long offset);


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
	long dataSize = ParseWaveHeader(fpWav, &formatChunkBuf);
	long dataTopPos = ftell(fpWav);
	long dataChunkTopPos = dataTopPos - 8;	// "data" + datasize
	long aligned_data_top = (dataTopPos + (0x1000 - 1)) & (~(0x1000 - 1));
	long offset = aligned_data_top - dataTopPos;

	struct stat st;
	fstat(fileno(fpWav),&st);
	long endPos = aligned_data_top + st.st_size - dataTopPos;
	long aligned_endPos = (endPos + (0x1000 - 1)) & (~(0x1000 - 1));
	long endPadSize = aligned_endPos - endPos;
	if ((endPadSize > 0) && (endPadSize < 8)) {
		aligned_endPos += 0x1000;
		endPadSize += 0x1000;
	}

	if (dataSize > 0) {
		CopyWavHeader(fpWav, fp, dataChunkTopPos, offset);
		MakePad(fp, offset);
		fseek(fpWav, dataChunkTopPos, SEEK_SET);
		CopyFile(fpWav, fp, -1);
		MakePad(fp, endPadSize);
	}

	fclose(fpWav);
	fclose(fp);
}


static void PutLongVal(unsigned long val_l, FILE* fp)
{
	for (int i = 0; i < 4; i++, (val_l >>= 8)) {
		fputc(val_l & 0xff, fp);
	}
}
static void PutShortVal(unsigned short val_s, FILE* fp)
{
	for (int i = 0; i < 2; i++, (val_s >>= 8)) {
		fputc(val_s & 0xff, fp);
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

void CopyWavHeader(FILE* fpWav, FILE* fp, long copySize, long offset)
{
	rewind(fpWav);
	CopyFile(fpWav, fp, copySize);

	fseek(fpWav, 4, SEEK_SET);
	long totalSize;
	fread(&totalSize, sizeof(long), 1, fpWav);

	totalSize += offset;

	fseek(fp, 4, SEEK_SET);
	fwrite(&totalSize, sizeof(long), 1, fp);
	fseek(fp, 0, SEEK_END);
}

void MakePad(FILE* fp, long offset)
{
	if (offset < 8) { return; }

	fputs("PAD ", fp);
	PutLongVal(offset - 8, fp);	// pad size
	for (int i = 0; i < offset - 8; i++) {
		fputc('\0', fp);
	}
}

