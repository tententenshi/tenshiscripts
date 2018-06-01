/**
 * Required library:
 *   The GNU Scientific Library for numerical analysis (gsl-0.6)
 *   http://sourceware.cygnus.com/gsl/
 *
 * How to compile:
 *   $ gcc -O2 -o dryconv dryconv.c -lgsl -lgslcblas -lm
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_fft_complex.h>
#include "../wav.h"

#define DEBUG_PRINT(...)    printf(__VA_ARGS__)

/* accessor for real and imaginary part of complex array */
#define REAL(z,i) ((z)[2*(i)])
#define IMAG(z,i) ((z)[2*(i)+1])

/* print debug information (-debug) */
bool debug = false;


void Usage(const char* command)
{
	fprintf(stderr, "make wave through FIR filters\n");
	fprintf(stderr, "%s input_fir_file input_wav_file output_wav_file\n\n", command);
	fprintf(stderr, "<Format of Input FIR file>\n");
	fprintf(stderr, "extension: *.txt, *.dat --> text (unity amount is 1.0)\n");
	fprintf(stderr, "           *.wav        --> binary, only monoral wav\n");
}

void ParseFIRFileAsWav(FILE *fp, double** pFIR_Data, int* pSize, int* pFFTSize);
void ParseFIRFileAsTxt(FILE *fp, double** pFIR_Data, int* pSize, int* pFFTSize);
void Process(FILE *fp, FILE *theWavFile, const SFormatChunk& formatChunk, int dataSize, const double* pFIR_Data, int theFIR_Size, int theFIR_FFTSize);
static void CopyFile(FILE* fpSrc, FILE* fpDst, int32_t copySize);

int main(int argc, char* argv[])
{
	if (argc != 4) {
		Usage(argv[0]);
		exit(1);
	}

	char* firFile = *++argv;
	char* inWav = *++argv;
	char* outFile = *++argv;

	FILE *fpWav, *fp, *fp2;

	if ((fpWav = fopen(inWav, "rb")) == NULL) {
		printf("Cannot open %s\n", inWav);
		exit(1);
	}
	if ((fp = fopen(firFile, "rb")) == NULL) {
		printf("Cannot open %s\n", firFile);
		exit(1);
	}
	if ((fp2 = fopen(outFile, "wb")) == NULL) {
		printf("Cannot open %s\n", outFile);
		exit(1);
	}

	SFormatChunk formatChunkBuf;
	int32_t dataSize = ParseWaveHeader(fpWav, &formatChunkBuf);
	int32_t dataStartPos = ftell(fpWav);
	rewind(fpWav);

	if (dataSize > 0) {
		double* pFIR_Data;
		int aFIR_Size, aFIR_FFTSize;

		if (strstr(firFile, ".wav") || strstr(firFile, ".WAV")) {
			ParseFIRFileAsWav(fp, &pFIR_Data, &aFIR_Size, &aFIR_FFTSize);
		} else {
			ParseFIRFileAsTxt(fp, &pFIR_Data, &aFIR_Size, &aFIR_FFTSize);
		}

		CopyFile(fpWav, fp2, dataStartPos);
//		SetupWavHead(fp2, formatChunkBuf, dataSize);
		Process(fp2, fpWav, formatChunkBuf, dataSize, pFIR_Data, aFIR_Size, aFIR_FFTSize);
		CopyFile(fpWav, fp2, -1);
	}

	fclose(fpWav);
	fclose(fp);
	fclose(fp2);
}

static void PutLongVal(uint32_t val_l, FILE* fp)
{
	for (int i = 0; i < 4; i++, (val_l >>= 8)) {
		fputc(val_l & 0xff, fp);
	}
}
static void PutShortVal(uint16_t val_s, FILE* fp)
{
	for (int i = 0; i < 2; i++, (val_s >>= 8)) {
		fputc(val_s & 0xff, fp);
	}
}


void ParseFIRFileAsWav(FILE *fp, double** pFIR_Data, int* pSize, int* pFFTSize)
{
	SFormatChunk formatChunk;
	int32_t dataSize = ParseWaveHeader(fp, &formatChunk);

	int num_ch = formatChunk.channel;
	if (num_ch != 1) {
		printf("FIR input file: num of channel is not 1\n");
		*pSize = 0;
		return;
	}

	int aSize = dataSize / formatChunk.blockSize;
	*pSize = aSize;
	double* pDataBuf = new double[aSize];
	*pFIR_Data = pDataBuf;
	int aFFTSize;
	for (aFFTSize = 1024; aFFTSize < aSize; aFFTSize *= 2) ; /* FFT size */
	*pFFTSize = aFFTSize;

	for (int i = 0; i < aSize; i++) {
		pDataBuf[i] = ReadWaveData(fp, formatChunk);
	}
}

void ParseFIRFileAsTxt(FILE *fp, double** pFIR_Data, int* pSize, int* pFFTSize)
{
	int aSize = 0;
	char aBuf[101];

	rewind(fp);
	while (1) {
		if (fgets(aBuf, 100, fp) == 0) { break; }
		double coef;
		int aColNum = sscanf(aBuf, "%lf", &coef);
		if (aColNum < 1) { continue; }	// skip empty line
		aSize++;
	}

	*pSize = aSize;
	double* pDataBuf = new double[aSize];
	*pFIR_Data = pDataBuf;
	int aFFTSize;
	for (aFFTSize = 1024; aFFTSize < aSize; aFFTSize *= 2) ; /* FFT size */
	*pFFTSize = aFFTSize;

	rewind(fp);
	int aCurCoef = 0;

	while (1) {
		if (fgets(aBuf, 100, fp) == 0) { break; }

		double coef;
		int aColNum = sscanf(aBuf, "%lf", &coef);
//		printf("%f %d\n", coef, aColNum);
		if (aColNum < 1) { continue; }	// skip empty line

		pDataBuf[aCurCoef] = coef;
		aCurCoef++;
	}
}


/************************************************************************
 * Algorithm
 * 
 * 1. Definition of convolution
 *   dry source signal : x(t) : t=0...N-1
 *   impulse response  : w(t) : t=0...M-1
 *   reproduced signal : y(t) : t=0...N+M-1
 * 
 *           M
 *   y(t) = \sum x(t-i) w(i)
 *          i=1
 * 
 *   Note: length M of impulse response should be 2の巾乗
 *         by addding zeros at the end of array.
 * 
 * 2. Convolution by FFT
 * 
 *  (1) Split dry source signal x(t) into signals x_k(t), k=1...K that 
 *      each length is M.
 * 
 *  (2) For each dry sources x_k(t), k=1...K and impulse response w(t), add zeros 
 *      at the end so that its length is 2M.
 * 
 *  (3) Apply FFT to each dry sources x_k(t) and impulse response w(t).
 * 
 *      X_k(f) = F[x_k(t)],    k=1...K
 *      W(f)   = F[w(t)],     
 * 
 *  (4) Multiply dry sources X_k(f) and impulse response W(f) at each frequency 
 *      to get reproduced signals Y_k(f).
 * 
 *      Y_k(f) = X_k(f) W(f),  f=0...2M-1, k=1...K
 * 
 *  (5) Apply inverse FFT to each reproduced signals Y_k(f).
 * 
 *      y_k(t) = F'[Y_k(f)],   k=1...K
 * 
 *  (6) Add all reproduced signals by shifting M as shown below.
 *      
 *                               ←             N          →
 *  dry source signal  x(t)     |*******|*******|*******|****---|  (*** : non-zero)
 *                               ← M → ← M → ← M → ← M →
 *		             
 *                               ← M → ← M →
 *  dry source signal1 x_1(t)   |*******|-------|
 *  impulse response   w(t)     |******-|-------|
 *  reproduced signal1 y_1(t)   |**************-|
 *                                       ← M → ← M →
 *  dry source signal2 x_2(t)           |*******|-------|
 *  impulse response   w(t)             |******-|-------|
 *  reproduced signal2 y_2(t)           |**************-|
 *                                               ← M → ← M →
 *  dry source signal3 x_3(t)                   |*******|-------|
 *  impulse response   w(t)                     |******-|-------|
 *  reproduced signal3 y_3(t)                   |**************-|
 *                                                       ← M → ← M →
 *  dry source signal4 x_4(t)                           |****---|-------|
 *  impulse response   w(t)                             |******-|-------|
 *  reproduced signal4 y_4(t)                           |***********----|
 *		             
 *  reproduced signal  y(t)     |*******|*******|*******|*******|***----|
 *                               ← M → ← M → ← M → ← M → ← M →
 * 
 ************************************************************************/
void Process(FILE *fp, FILE *theWavFile, const SFormatChunk& formatChunk, int dataSize, const double* pFIR_Data, int theFIR_Size, int theFFTSize)
{
	int wavSamples = dataSize / formatChunk.blockSize;
	int FSAMP = formatChunk.fsamp;
	int num_ch = formatChunk.channel;

	int aSrcSize = dataSize / formatChunk.blockSize;

	double** srcBuf = new double*[num_ch];
	for (int ch = 0; ch < num_ch; ch++) {
		srcBuf[ch] = new double[theFFTSize];
	}

	DEBUG_PRINT("Dry source      : %6d[pts]\n", dataSize);
	DEBUG_PRINT("Impulse response: %6d[pts]\n", theFIR_Size, pFIR_Data);
	DEBUG_PRINT("FFT size        : %6d[pts]\n", theFFTSize);

	/* normalization factor for impulse response */
	double power = 0;		/* total power of impulse response */
	for (int i = 0; i < theFIR_Size; i++ ) power += pFIR_Data[i] * pFIR_Data[i];
	power = sqrt(power);
	DEBUG_PRINT("Power of impulse: %.3g\n",power);

	/* alloc FFT buffers */
	double* impfft = new double[(2*theFFTSize) * 2];
	double** dryfft = new double*[num_ch];
	for (int ch = 0; ch < num_ch; ch++) {
		dryfft[ch] = new double[(2*theFFTSize) * 2];
	}
	double** outfft = new double*[num_ch];
	for (int ch = 0; ch < num_ch; ch++) {
		outfft[ch] = new double[(2*theFFTSize) * 2];
	}

	/* preparation of impulse response */
	for (int i = 0;           i < theFIR_Size;  i++ ) REAL(impfft,i) = pFIR_Data[i] / power;
	for (int i = theFIR_Size; i < 2*theFFTSize; i++ ) REAL(impfft,i) = 0;
	for (int i = 0;           i < 2*theFFTSize; i++ ) IMAG(impfft,i) = 0;

	/* FFT of impulse response */
	gsl_fft_complex_radix2_forward (impfft, 1, 2*theFFTSize); 


	/* preparation of output reproduced signal */
	int outlen = (aSrcSize / theFFTSize + 2) * theFFTSize;
	double** outdata = new double*[num_ch];
	for (int ch = 0; ch < num_ch; ch++) {
		outdata[ch] = new double[outlen];
		for (int i = 0; i < outlen; i++) outdata[ch][i] = 0;
	}

	/**
	 * convolution iteration
	 */
	for (int mm = 0; mm < aSrcSize; mm += theFFTSize) {
		for (int i = 0; i < theFFTSize && mm+i < aSrcSize; i++) {
			for (int ch = 0; ch < num_ch; ch++) {
				srcBuf[ch][i] = ReadWaveData(theWavFile, formatChunk);
			}
		}
		for (int ch = 0; ch < num_ch; ch++) {
			/* preparation of dry source */
			for (int i = 0; i < 2*theFFTSize; i++ ) REAL(dryfft[ch],i) = IMAG(dryfft[ch],i) = 0;
			for (int i = 0; i < theFFTSize && mm+i < aSrcSize; i++ ) REAL(dryfft[ch],i) = srcBuf[ch][i];

			/* FFT of dry source */
			gsl_fft_complex_radix2_forward (dryfft[ch], 1, 2*theFFTSize); 

			/* the product of dry source and impulse response at each frequency */
			for (int i = 0; i < 2*theFFTSize; i++) {
				REAL(outfft[ch],i) = REAL(dryfft[ch],i) * REAL(impfft,i) - IMAG(dryfft[ch],i) * IMAG(impfft,i);
				IMAG(outfft[ch],i) = REAL(dryfft[ch],i) * IMAG(impfft,i) + IMAG(dryfft[ch],i) * REAL(impfft,i);
			}

			/* inverse FFT */
			gsl_fft_complex_radix2_inverse (outfft[ch], 1, 2*theFFTSize);

			/* add to reproduced signal */
			for (int i = 0; i < 2*theFFTSize; i++) outdata[ch][mm+i] += (int16_t)REAL(outfft[ch],i);
		}
	}

	/**
	 * Write to float format file
	 */
//	for (int i = 0; i < aSrcSize + theFIR_Size; i++) {
	for (int i = 0; i < aSrcSize; i++) {
		for (int ch = 0; ch < num_ch; ch++) {
			WriteWaveData(fp, formatChunk, outdata[ch][i]);
		}
	}

	for (int ch = 0; ch < num_ch; ch++) {
		delete [] srcBuf[ch];
		delete [] outfft[ch];
		delete [] outdata[ch];
	}
	delete [] srcBuf;
	delete [] impfft;
	delete [] outfft;
	delete [] outdata;
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

