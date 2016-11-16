//
// BBD device emulator  ( not delay emulator )
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../wav.h"


//static double ChorusFreqRatio = 1.005;
static double ChorusFreqRatio = 1.02;
static double MinBBDClock =  83000;		// Hz
static double MaxBBDClock = 400000;		// Hz


enum { NUM_OF_MAX_CH = 2, };
//enum { BBD_LENGTH = 0x10000, };		// same as MN-3007
enum { BBD_LENGTH = 0x400, };		// same as MN-3007
double	buf_441[NUM_OF_MAX_CH][16], buf_bbd[NUM_OF_MAX_CH][BBD_LENGTH];		// delay buf

void	init(void);
double	proc(int ch, double val, const SFormatChunk& formatChunk);
void	write_bbd(int ch, int bp_441, double pha4, int bp_bbd);
double	read_bbd(int ch, int bp_bbd, double phaf_bbd);
void	write_441(int ch, double val, int bp_441);
double	lerp(double, double, double);
static void CopyFile(FILE* fpSrc, FILE* fpDst, long copySize);


// --------------------------------------------------------------------
void Usage(const char* command)
{
	fprintf(stderr, "BBD simulator\n");
	fprintf(stderr, "%s input_wav_file output filename.wav\n\n", command);
}

// --------------------------------------------------------------------
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

	if ((fpWav = fopen(inWav, "r")) == NULL) {
		printf("Cannot open %s\n", inWav);
		exit(1);
	}
	if ((fpOut = fopen(outfile, "w")) == NULL) {
		printf("Cannot open %s\n", outfile);
		exit(1);
	}

	SFormatChunk formatChunkBuf;
	long dataSize = ParseWaveHeader(fpWav, &formatChunkBuf);
	long dataStartPos = ftell(fpWav);
	rewind(fpWav);

	if (dataSize > 0) {
		init();

		CopyFile(fpWav, fpOut, dataStartPos);
//		SetupWavHead(fpOut, formatChunkBuf, dataSize);

		{
			int wavSamples = dataSize / formatChunkBuf.blockSize;
			int FSAMP = formatChunkBuf.fsamp;
			int num_ch = formatChunkBuf.channel;

			for (int i = 0; i < wavSamples; i++) {
				for (int ch = 0; ch < num_ch; ch++) {
					double val = ReadWaveData(fpWav, &formatChunkBuf);
		
//					printf("%4d ", i);
					double new_val = proc(ch, val, formatChunkBuf);

//					WriteWaveData(fpOut, &formatChunkBuf, new_val);
					WriteWaveData(fpOut, &formatChunkBuf, 0.5 * val + 0.5 * new_val);
				}
			}
		}

		CopyFile(fpWav, fpOut, -1);
	}

	fclose(fpWav);
	fclose(fpOut);
}

// --------------------------------------------------------------------
double proc(int ch, double val, const SFormatChunk& formatChunk)
{
	double	inc_bbd = 1.0;					// bbd phase inc ( 1.0: 44.1k system sample rate )

	static double ChorusLFO = 0;
#if 1
	int FSAMP = formatChunk.fsamp;
	static double aPeriod = 1 / MaxBBDClock * (MaxBBDClock / MinBBDClock - 1.0) / (pow(ChorusFreqRatio, 1.0 / BBD_LENGTH) - 1.0);
	ChorusLFO += 1.0 / (aPeriod * FSAMP);
	if (ChorusLFO > 1.0) { ChorusLFO -= 2.0; }

	double aCurPoint = fabs(ChorusLFO);
	double TargetBBDClock = 1 / ((1/MaxBBDClock - 1/MinBBDClock) * aCurPoint + 1/MinBBDClock);
	inc_bbd = TargetBBDClock / FSAMP;
#else
	inc_bbd = 0.2;
#endif

	double inv_inc = 1.0 / inc_bbd;

	static volatile double	phaf_bbd = 0.0;					// float phase
	static volatile int		bp_441 = 0, bp_bbd = 0;				// base ptr ( dec by sample(441) / dec by write(bbd) )

	double rd = read_bbd(ch, bp_bbd, phaf_bbd);												// SRC bbd -> 441
//	printf("     read_bbd   inc: %9.6lf,  bp_441: %d,  phaf: %g,  bp_bbd: %d,  mxo: %g\n",
//		   inc_bbd,        bp_441,     phaf_bbd,       bp_bbd,   rd);

	double bbd_index_temp = phaf_bbd + inc_bbd;							// post proc / update for next sample period
	int phai_bbd = (int)bbd_index_temp;						// integer part of phase

	static double	buf_inc_bbd[BBD_LENGTH];	// for debug "inc_bbd" modulation

	for (int i = 0; i < phai_bbd; i++) {
		double tmp_441 = 1.0 - (i + (1.0 - phaf_bbd)) * inv_inc;	// assuming fixed inc_bbd while one 441kHz sampling period, interpolate inc_bbd as needed
		write_bbd(ch, bp_441, tmp_441, bp_bbd);
//		printf("     write_bbd  inc: %g,  bp_441: %d,  tmp_441: %g, phaf: %g, bbd_index_temp: %g, i: %d,  bp_bbd: %d,  buf_bbd: %g\n",
//			   inc_bbd,        bp_441,     tmp_441, phaf_bbd, bbd_index_temp, i, bp_bbd,       buf_bbd[ch][bp_bbd]);
		buf_inc_bbd[bp_bbd] = inc_bbd;	// for debug "inc_bbd" modulation
		bp_bbd = --bp_bbd & (BBD_LENGTH - 1);
	}
	phaf_bbd = bbd_index_temp - phai_bbd;

	{
		// for debug "inc_bbd" modulation
		int bp_read = bp_bbd + (BBD_LENGTH - 4);											// 0x1fc: bbd cell count(BBD_LENGTH) - some latency@input
		double inc_bbd_on_wr = buf_inc_bbd[bp_read & (BBD_LENGTH - 1)];	// for debug
		if (inc_bbd_on_wr == 0) { inc_bbd_on_wr = 1.0; }
//		printf("ChorusLFO: %g, TargetBBDClock: %g, inc_bbd_wr: %g, inc_bbd: %g, ratio: %g, phai_bbd: %d, phaf_bbd: %g, bp_bbd: %d\n", ChorusLFO, TargetBBDClock, inc_bbd_on_wr, inc_bbd, inc_bbd / inc_bbd_on_wr, phai_bbd, phaf_bbd, bp_bbd);
	}

	write_441(ch, val, bp_441);
//	printf("---- write_441  inc: %9.6lf  bp_441: %01x  phab: %9.6lf  bp_bbd: %03x  buf_441\[%01x]=%9.6lf\n",
//								 inc_bbd,        bp_441,     phaf_bbd,       bp_bbd,        bp_441, dat_flg ? mxi : 0.0);
	bp_441 = --bp_441 & 0xf;
	return rd;
}

// --------------------------------------------------------------------
void init(void)
{
	for (int ch = 0; ch < NUM_OF_MAX_CH; ch++) {
		for (int i=0; i<0x10; i++) {
			buf_441[ch][i] = 0.0;
		}
		for (int i=0; i<BBD_LENGTH; i++) {
			buf_bbd[ch][i] = 0.0;
		}
	}
}
// --------------------------------------------------------------------
void write_bbd(int ch, int bp_441, double pha4, int bp_bbd)
{
	int bp = bp_441 + 2;												// 2: offset for interpolation
	double wd = lerp(buf_441[ch][(bp - 1) & 0xf], buf_441[ch][bp & 0xf], pha4);
	buf_bbd[ch][bp_bbd] = wd;
}

// --------------------------------------------------------------------
double lerp(double d0, double d1, double k)
{
	return d0 * (1.0 - k) + d1 * k;
}

// --------------------------------------------------------------------
double read_bbd(int ch, int bp_bbd, double phaf_bbd)
{
	int bp = bp_bbd + (BBD_LENGTH - 4);											// 0x1fc: bbd cell count(BBD_LENGTH) - some latency@input
	double rd = lerp(buf_bbd[ch][(bp - 1) & (BBD_LENGTH - 1)], buf_bbd[ch][bp & (BBD_LENGTH - 1)], (1.0 - phaf_bbd));
//	aaf((inc_bbd > 1.0) ? 1 / inc_bbd : 1.0);							// anti arias filter
	return rd;
}
// --------------------------------------------------------------------
void write_441(int ch, double val, int bp_441)
{
//	aaf((inc_bbd < 1.0) ? inc_bbd : 1.0);							// anti arias flter
	buf_441[ch][bp_441] = val;
}

// --------------------------------------------------------------------
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

// --------------------------------------------------------------------
