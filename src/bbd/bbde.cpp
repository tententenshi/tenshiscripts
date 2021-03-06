//
// BBD device emulator  ( not delay emulator )
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../wav.h"


static double ChorusFreqRatio = 1.02;
//static double ChorusFreqRatio = 1.2;
static double MinBBDClock =  83000;		// Hz
static double MaxBBDClock = 400000;		// Hz
static double sModulationPeriod;

enum { NUM_OF_MAX_CH = 2, };
//enum { BBD_LENGTH = 0x10000, };		// same as MN-3007
enum { BBD_LENGTH = 0x400, };		// same as MN-3007
double	buf_bbd[NUM_OF_MAX_CH][BBD_LENGTH * 2];		// delay buf
static double	buf_441[NUM_OF_MAX_CH][4];

void	init(void);
double	proc(int ch, double val, const SFormatChunk& formatChunk);
double	proc_only_interpolationread(int ch, double val, const SFormatChunk& formatChunk);
double	interpolated_read(const double*, double);
void	proceed_sample(int ch);
static void CopyFile(FILE* fpSrc, FILE* fpDst, int32_t copySize);

static double sCurReadSpead;
static double sCurWriteSpead;
static double sCurPitch;
static double sCurWriteAdrs;
static double sCurDelayTime;
//static double sCurPitch2;

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

	eEndian aEndian = ConfirmSystemEndian();
	if (aEndian == eBIG) {
		printf("this system is big endian\n");
	} else if (aEndian == eLITTLE) {
		printf("this system is little endian\n");
	} else {
		printf("this system is unknown\n");
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

	SFormatChunk formatChunkBuf;
	int32_t dataSize = ParseWaveHeader(fpWav, &formatChunkBuf);
	int32_t dataStartPos = ftell(fpWav);
//	rewind(fpWav);

	if (dataSize > 0) {
		init();

//		CopyFile(fpWav, fpOut, dataStartPos);

		int32_t sOutFileDataStartPos;
		SFormatChunk formatChunkOut;

		enum { DEBUG_ITEM_NUM = 4, };
		{
			int num_ch = formatChunkBuf.channel + DEBUG_ITEM_NUM;
			int FSAMP = formatChunkBuf.fsamp;
			int bitLength = 32;
			int format = 3;

			sOutFileDataStartPos = CreateWavHeader(fpOut, &formatChunkOut, num_ch, FSAMP, bitLength, format);
		}

		{
			int wavSamples = dataSize / formatChunkBuf.blockSize;
			int FSAMP = formatChunkBuf.fsamp;
			int num_ch = formatChunkBuf.channel;

			for (int i = 0; i < wavSamples; i++) {
				for (int ch = 0; ch < num_ch; ch++) {
					double val = ReadWaveData(fpWav, &formatChunkBuf);
		
//					printf("%4d ", i);
#if defined(ORIGINAL_ALG)
					double new_val = proc(ch, val, formatChunkBuf);
#else
					double new_val = proc_only_interpolationread(ch, val, formatChunkBuf);
#endif

					WriteWaveData(fpOut, &formatChunkOut, new_val);
//					WriteWaveData(fpOut, &formatChunkOut, 0.5 * val + 0.5 * new_val);
				}
#if 1
				WriteWaveData(fpOut, &formatChunkOut, sCurReadSpead / 10.0);
				WriteWaveData(fpOut, &formatChunkOut, sCurWriteSpead / 10.0);
				WriteWaveData(fpOut, &formatChunkOut, (sCurPitch - 1.0) * 40);
//				WriteWaveData(fpOut, &formatChunkOut, sCurWriteAdrs / BBD_LENGTH);
				WriteWaveData(fpOut, &formatChunkOut, sCurDelayTime / BBD_LENGTH);
#endif
			}
		}

//		CopyFile(fpWav, fpOut, -1);

		{
			int aDataSize = dataSize / formatChunkBuf.blockSize * formatChunkOut.blockSize;
			MaintainWavHeader(fpOut, aDataSize, sOutFileDataStartPos);
		}
	}

	fclose(fpWav);
	fclose(fpOut);
}

static inline double GetPhaseIncrement(double theCurPoint, double FSAMP)
{
	double TargetBBDClock = 1 / ((1/MaxBBDClock - 1/MinBBDClock) * theCurPoint + 1/MinBBDClock);	// reciplocal
//	double TargetBBDClock = (MaxBBDClock - MinBBDClock) * theCurPoint + MinBBDClock;	// linear
//	double TargetBBDClock = MinBBDClock * exp(log(MaxBBDClock / MinBBDClock) * theCurPoint);	// exponential
//	double TargetBBDClock = MinBBDClock + (MaxBBDClock - MinBBDClock) * theCurPoint * theCurPoint;	// square
	return TargetBBDClock / FSAMP;
}

// --------------------------------------------------------------------
double proc(int ch, double val, const SFormatChunk& formatChunk)
{
	static double ChorusLFO[NUM_OF_MAX_CH] = {0};
#if 1
	int FSAMP = formatChunk.fsamp;
	ChorusLFO[ch] += 1.0 / (sModulationPeriod * FSAMP);
	if (ChorusLFO[ch] > 1.0) { ChorusLFO[ch] -= 2.0; }

	double aCurPoint = fabs(ChorusLFO[ch]);
	double inc_bbd = GetPhaseIncrement(aCurPoint, FSAMP);
#else
	double inc_bbd = 0.2;	// phase increment
#endif

	double inv_inc = 1.0 / inc_bbd;	// inverse of phase increment

	static volatile double	bp_bbd[NUM_OF_MAX_CH] = {BBD_LENGTH};			// float phase

	buf_441[ch][3] = val;

	double rp = bp_bbd[ch] - BBD_LENGTH + 4;											// 0x1fc: bbd cell count(BBD_LENGTH) - some latency@input
	double rd = interpolated_read(buf_bbd[ch], rp);
//	printf(" READ_bbd inc: %lf, rp: %lf, bbd_val[0]: %lf, bbd_val[1]: %lf, rd: %lf\n", inc_bbd, rp, buf_bbd[ch][(int)rp+0], buf_bbd[ch][(int)rp+1], rd);

	static double	buf_inc_bbd[NUM_OF_MAX_CH][BBD_LENGTH * 2];	// for debug "inc_bbd" modulation
	static double	buf_time_history_bbd[NUM_OF_MAX_CH][BBD_LENGTH * 2];	// for debug
	static int sCurTime[NUM_OF_MAX_CH] = {0};	// for debug
	{
		sCurTime[ch]++;
//		static double sPrevReadTime;
		// for debug "inc_bbd" modulation
		double inc_bbd_on_wr = interpolated_read(buf_inc_bbd[ch], rp);	// for debug
		double time_on_wr = interpolated_read(buf_time_history_bbd[ch], rp);	// for debug
//		printf("ChorusLFO: %lf, TargetBBDClock: %lf, inc_bbd_wr: %lf, inc_bbd: %lf, ratio: %lf, bp_bbd: %lf\n", ChorusLFO[ch], TargetBBDClock, inc_bbd_on_wr, inc_bbd, inc_bbd / inc_bbd_on_wr, bp_bbd[ch] - BBD_LENGTH);
//		printf("ChorusLFO: %lf, TargetBBDClock: %lf, inc_bbd_wr: %lf, inc_bbd: %lf, ratio: %lf, bp_bbd: %lf, time_on_wr: %lf\n", ChorusLFO[ch], TargetBBDClock, inc_bbd_on_wr, inc_bbd, inc_bbd / inc_bbd_on_wr, bp_bbd[ch] - BBD_LENGTH, time_on_wr);
		sCurReadSpead = inc_bbd;
		sCurWriteSpead = inc_bbd_on_wr;
		sCurPitch = inc_bbd / inc_bbd_on_wr;
		sCurWriteAdrs = bp_bbd[ch] - BBD_LENGTH;
//		sCurPitch2 = (time_on_wr - sPrevReadTime);	// pitch is delta of delay time, simple analysis
//		sPrevReadTime = time_on_wr;
		sCurDelayTime = sCurTime[ch] - time_on_wr;
	}

	double bp_new = bp_bbd[ch] + inc_bbd;							// post proc / update for next sample period
	int int_adrs_prev = (int)bp_bbd[ch];
	double frac_adrs_prev = bp_bbd[ch] - int_adrs_prev;
	int int_adrs_new = (int)bp_new;

	for (int cur_int_adrs = int_adrs_prev + 1; cur_int_adrs <= int_adrs_new; cur_int_adrs++) {
		double adrs_441_frac = (cur_int_adrs - bp_bbd[ch]) * inv_inc;	// assuming fixed inc_bbd while one 441kHz sampling period, interpolate inc_bbd as needed
		double wd = interpolated_read(buf_441[ch], adrs_441_frac);
		int cur_int_adrs_wrapped = cur_int_adrs % BBD_LENGTH;
		buf_bbd[ch][cur_int_adrs_wrapped + BBD_LENGTH] = buf_bbd[ch][cur_int_adrs_wrapped] = wd;
//		printf(" write_bbd inc: %lf,  val[0]: %lf, val[1]: %lf, adrs_441_frac: %lf, bp_bbd: %lf, bp_new: %lf, cur_int_adrs_wrapped: %d,  buf_bbd: %lf\n",
//			inc_bbd, buf_441[ch][0], buf_441[ch][1], adrs_441_frac, bp_bbd[ch], bp_new, cur_int_adrs_wrapped, buf_bbd[ch][cur_int_adrs_wrapped]);
		buf_inc_bbd[ch][cur_int_adrs_wrapped + BBD_LENGTH] = buf_inc_bbd[ch][cur_int_adrs_wrapped] = inc_bbd;	// for debug "inc_bbd" modulation
		buf_time_history_bbd[ch][cur_int_adrs_wrapped + BBD_LENGTH] = buf_time_history_bbd[ch][cur_int_adrs_wrapped] = sCurTime[ch] + adrs_441_frac;	// for debug "inc_bbd" modulation
	}
	bp_bbd[ch] = (bp_new >= 2 * BBD_LENGTH) ? (bp_new - BBD_LENGTH) : bp_new;

	proceed_sample(ch);
	return rd;
}






double proc_only_interpolationread(int ch, double val, const SFormatChunk& formatChunk)
{
	static double ChorusLFO[NUM_OF_MAX_CH] = {0};
#if 1
	int FSAMP = formatChunk.fsamp;
	ChorusLFO[ch] += 1.0 / (sModulationPeriod * FSAMP);
	if (ChorusLFO[ch] > 1.0) { ChorusLFO[ch] -= 2.0; }

	double aCurPoint = fabs(ChorusLFO[ch]);
	double inc_bbd = GetPhaseIncrement(aCurPoint, FSAMP);
#else
	double inc_bbd = 1.0;	//0.2;	// phase increment
#endif

	static double	rp_buf[NUM_OF_MAX_CH];			// float phase
	static int wp_buf[NUM_OF_MAX_CH];

	double rp = rp_buf[ch] + 4;											// 0x1fc: bbd cell count(BBD_LENGTH) - some latency@input
	double rd = interpolated_read(buf_bbd[ch], rp);
//	printf(" READ_bbd inc: %lf, rp: %lf, bbd_val[0]: %lf, bbd_val[1]: %lf, rd: %lf\n", inc_bbd, rp, buf_bbd[ch][(int)rp+0], buf_bbd[ch][(int)rp+1], rd);

	static double	buf_inc_bbd[NUM_OF_MAX_CH][BBD_LENGTH * 2];	// for debug "inc_bbd" modulation
	static double	buf_time_history_bbd[NUM_OF_MAX_CH][BBD_LENGTH * 2];	// for debug
	static int sCurTime[NUM_OF_MAX_CH] = {0};	// for debug
	{
		sCurTime[ch]++;
		// for debug "inc_bbd" modulation
		double inc_bbd_on_wr = interpolated_read(buf_inc_bbd[ch], rp);	// for debug
		double time_on_wr = interpolated_read(buf_time_history_bbd[ch], rp);	// for debug
//		printf("ChorusLFO: %lf, inc_bbd_wr: %lf, inc_bbd: %lf, ratio: %lf, wp_buf: %d, rp_buf: %lf\n", ChorusLFO[ch], inc_bbd_on_wr, inc_bbd, inc_bbd / inc_bbd_on_wr, wp_buf[ch], rp_buf[ch]);
//		printf("ChorusLFO: %lf, TargetBBDClock: %lf, inc_bbd_wr: %lf, inc_bbd: %lf, ratio: %lf, bp_bbd: %lf, time_on_wr: %lf\n", ChorusLFO[ch], TargetBBDClock, inc_bbd_on_wr, inc_bbd, inc_bbd / inc_bbd_on_wr, rp_buf[ch], time_on_wr);
		sCurReadSpead = inc_bbd;
		sCurWriteSpead = inc_bbd_on_wr;
		sCurPitch = (inc_bbd_on_wr <= 0) ? inc_bbd : (inc_bbd / inc_bbd_on_wr);
		sCurDelayTime = sCurTime[ch] - time_on_wr;
	}

	double rp_new = rp_buf[ch] + sCurPitch;							// post proc / update for next sample period
	rp_new = (rp_new >= BBD_LENGTH) ? (rp_new - BBD_LENGTH) : (rp_new < 0) ? (rp_new + BBD_LENGTH) : rp_new;
	int wp_new = wp_buf[ch] + 1;
	wp_new = (wp_new >= BBD_LENGTH) ? (wp_new - BBD_LENGTH) : (wp_new < 0) ? (wp_new + BBD_LENGTH) : wp_new;

	buf_bbd[ch][wp_new + BBD_LENGTH] = buf_bbd[ch][wp_new] = val;
	buf_inc_bbd[ch][wp_new + BBD_LENGTH] = buf_inc_bbd[ch][wp_new] = inc_bbd;	// for debug "inc_bbd" modulation
	buf_time_history_bbd[ch][wp_new + BBD_LENGTH] = buf_time_history_bbd[ch][wp_new] = sCurTime[ch];	// for debug "inc_bbd" modulation

	rp_buf[ch] = rp_new;
	wp_buf[ch] = wp_new;

	proceed_sample(ch);
	return rd;
}

// --------------------------------------------------------------------



// --------------------------------------------------------------------
void init(void)
{
	for (int ch = 0; ch < NUM_OF_MAX_CH; ch++) {
		for (int i=0; i<BBD_LENGTH; i++) {
			buf_bbd[ch][i] = 0.0;
		}
	}

	sModulationPeriod = 1 / MaxBBDClock * (MaxBBDClock / MinBBDClock - 1.0) / (pow(ChorusFreqRatio, 1.0 / BBD_LENGTH) - 1.0);
	printf("calculated modulation frequency: %lf [Hz] @pitch ratio: %lf, MinBBDClock: %lf, MaxBBDClock: %lf\n", 1 / sModulationPeriod, ChorusFreqRatio, MinBBDClock, MaxBBDClock);
}

// --------------------------------------------------------------------
double interpolated_read(const double* data, double address)
{
	int adrs_int = (int)address;
	float k = address - adrs_int;
#if 1
	return data[adrs_int] * (1.0 - k) + data[adrs_int + 1] * k;	// linear interpolation
#else
	double val = 0;
	for (int i = 0; i < 4; i++) {
		double x = -2 + i + k;
		double window = 0.4668 + 0.4832 * cos(M_PI * x / 2) + 0.05 * cos(M_PI * x);
		double sinc = (x != 0) ? (sin(M_PI * x / 2) / (M_PI * x / 2)) : 1;
		val += data[adrs_int + i] * 0.6533 * window * sinc;
//		printf("i: %d, k: %g, window: %g, sinc: %g, val: %g\n", i, k, window, sinc, val);
	}
	return val;
#endif
}

// --------------------------------------------------------------------
void proceed_sample(int ch)
{
	buf_441[ch][0] = buf_441[ch][1];
	buf_441[ch][1] = buf_441[ch][2];
	buf_441[ch][2] = buf_441[ch][3];
	buf_441[ch][3] = 0;
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
