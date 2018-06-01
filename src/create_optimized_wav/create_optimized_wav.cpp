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

static void CopyFile(FILE* fpSrc, FILE* fpDst, int32_t copySize);
static void CopyWavHeader(FILE* fpWav, FILE* fp, int32_t copySize, int32_t offset);
static void MakePad(FILE* fp, int32_t offset);


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

	if ((fpWav = fopen(inWav, "rb")) == NULL) {
		printf("Cannot open %s\n", inWav);
		exit(1);
	}
	if ((fp = fopen(outfile, "wb")) == NULL) {
		printf("Cannot open %s\n", outfile);
		exit(1);
	}

	SFormatChunk formatChunkBuf;
	int32_t dataSize = ParseWaveHeader(fpWav, &formatChunkBuf);
	int32_t dataTopPos = ftell(fpWav);
	int32_t dataChunkTopPos = dataTopPos - 8;	// "data" + datasize
	int32_t aligned_data_top = (dataTopPos + (0x1000 - 1)) & (~(0x1000 - 1));
	int32_t offset = aligned_data_top - dataTopPos;

	struct stat st;
	fstat(fileno(fpWav),&st);
	int32_t endPos = aligned_data_top + st.st_size - dataTopPos;
	int32_t aligned_endPos = (endPos + (0x1000 - 1)) & (~(0x1000 - 1));
	int32_t endPadSize = aligned_endPos - endPos;
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

void CopyWavHeader(FILE* fpWav, FILE* fp, int32_t copySize, int32_t offset)
{
	rewind(fpWav);
	CopyFile(fpWav, fp, copySize);

	fseek(fpWav, 4, SEEK_SET);
	int32_t totalSize;
	fread(&totalSize, sizeof(int32_t), 1, fpWav);

	totalSize += offset;

	fseek(fp, 4, SEEK_SET);
	fwrite(&totalSize, sizeof(int32_t), 1, fp);
	fseek(fp, 0, SEEK_END);
}

void MakePad(FILE* fp, int32_t offset)
{
	if (offset < 8) { return; }

	fputs("PAD ", fp);
	PutLongVal(offset - 8, fp);	// pad size
	for (int i = 0; i < offset - 8; i++) {
		fputc('\0', fp);
	}
}

