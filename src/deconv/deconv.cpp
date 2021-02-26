#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex>
#include "../wav.h"


static int sFFTLength;
static int sIR_Channel;
static double** ir_fft_data;

static void ParseIrWaveFile(FILE* FP_IrWaveFile, SFormatChunk* aFormat);
static void ProcessWaveFile(FILE* FP_SrcWaveFile, FILE* FP_OutWaveFile);
static void CopyFile(FILE* fpSrc, FILE* fpDst, int32_t copySize);
extern "C" void cdft(int n, int isgn, double *a, int *ip, double *w);


int main(int argc, char *argv[])
{
	if (argc != 4) {
		printf("usage %s IrWaveFile SrcWaveFile OutWaveFile\n", argv[0]);
		exit(1);
	}
	const char* aIrWaveFile = argv[1];
	const char* aSrcWaveFile = argv[2];
	const char* aOutWaveFile = argv[3];

	FILE* FP_IrWaveFile = fopen(aIrWaveFile, "rb");
	FILE* FP_SrcWaveFile = fopen(aSrcWaveFile, "rb");
	FILE* FP_OutWaveFile = fopen(aOutWaveFile, "wb");

	if ((FP_IrWaveFile == NULL) || (FP_SrcWaveFile == NULL) || (FP_OutWaveFile == NULL)) {
		printf("file open error\n");
		exit(1);
	}

	printf("file opened\n");

	SFormatChunk formatChunkBuf_IrWave;

	ParseIrWaveFile(FP_IrWaveFile, &formatChunkBuf_IrWave);
	ProcessWaveFile(FP_SrcWaveFile, FP_OutWaveFile);

	fclose(FP_IrWaveFile);
	fclose(FP_SrcWaveFile);
	fclose(FP_OutWaveFile);

	for (int ch = 0; ch < formatChunkBuf_IrWave.channel; ch++) {
		delete[] ir_fft_data[ch];
	}
	delete[] ir_fft_data;
}


/* accessor for real and imaginary part of complex array */
#define REAL(z,i) ((z)[2*(i)])
#define IMAG(z,i) ((z)[2*(i)+1])

static void ParseIrWaveFile(FILE* FP_IrWaveFile, SFormatChunk* aFormat)
{
	int aDataSize = ParseWaveHeader(FP_IrWaveFile, aFormat);

	int wavSamples = aDataSize / aFormat->blockSize;
	sIR_Channel = aFormat->channel;

	int aFFTBit = (int)(ceil(log(wavSamples) / log(2.0)));
	sFFTLength = 1 << (aFFTBit + 1);	// FFT length is twice of IR wave length

	printf("samples: %d and fft length: %d\n", wavSamples, sFFTLength);

	double** dataBuf = new double*[sIR_Channel];
	for (int ch = 0; ch < sIR_Channel; ch++) {
		dataBuf[ch] = new double[wavSamples];
	}

	for (int i = 0; i < wavSamples; i++) {
		for (int ch = 0; ch < sIR_Channel; ch++) {
			dataBuf[ch][i] = ReadWaveData(FP_IrWaveFile, aFormat);
		}
	}

	double* power = new double[sIR_Channel];
	for (int ch = 0; ch < sIR_Channel; ch++) {
		power[ch] = 0;
		for (int sample = 0; sample < wavSamples; sample++) {
			power[ch] += dataBuf[ch][sample] * dataBuf[ch][sample];
		}
		power[ch] = sqrt(power[ch]);
		printf("power of impulse (ch %d): %.3g\n", ch, power[ch]);
		if (power[ch] == 0) { printf("power of impulse is zero\n"); exit(1); }
	}

	double powerMax = 0;
	for (int ch = 0; ch < sIR_Channel; ch++) {
		powerMax = fmax(powerMax, power[ch]);
	}
	if (powerMax == 0) { printf("power of impulse is zero\n"); exit(1); }

	// memory allocation
	int n = sFFTLength * 2;
	int* ip = new int[2 + (int)(ceil(sqrt(n)))];
	double* w = new double[sFFTLength];
	ir_fft_data = new double*[sIR_Channel];
	for (int ch = 0; ch < sIR_Channel; ch++) {
		ir_fft_data[ch] = new double[n];

		/* preparation of impulse response */
		for (int i = 0; i < wavSamples; i++) { REAL(ir_fft_data[ch], i) = dataBuf[ch][i]; }
		for (int i = wavSamples; i < sFFTLength; i++) { REAL(ir_fft_data[ch], i) = 0; }		// zero padding for linear convolution
		for (int i = 0; i < sFFTLength; i++) { IMAG(ir_fft_data[ch], i) = 0; }
	}

	for (int ch = 0; ch < sIR_Channel; ch++) {
		ip[0] = 0;
		cdft(n, -1, ir_fft_data[ch], ip, w);
	}

	for (int ch = 0; ch < sIR_Channel; ch++) {
		for (int i = 0; i < 64; i++) {
			std::complex<double> aIR(REAL(ir_fft_data[ch], i), IMAG(ir_fft_data[ch], i));
			double aMag = abs(aIR);
			double aArg = arg(aIR);
			printf("%d mag %g arg %g\n", i, aMag, aArg * 360 / (2 * M_PI));
		}
	}

	for (int ch = 0; ch < sIR_Channel; ch++) {
		delete[] dataBuf[ch];
	}
	delete[] power;
	delete[] dataBuf;
	delete[] ip;
	delete[] w;
}

static void ProcessWaveFile(FILE* FP_SrcWaveFile, FILE* FP_OutWaveFile)
{
	SFormatChunk formatChunkBuf;
	int32_t dataSize = ParseWaveHeader(FP_SrcWaveFile, &formatChunkBuf);
	int32_t dataStartPos = ftell(FP_SrcWaveFile);
	rewind(FP_SrcWaveFile);

	if (dataSize > 0) {
		CopyFile(FP_SrcWaveFile, FP_OutWaveFile, dataStartPos);
//		SetupWavHead(fp2, formatChunkBuf, dataSize);

		// process data
		int wavSamples = dataSize / formatChunkBuf.blockSize;
		int num_ch = formatChunkBuf.channel;

		double** dataBuf;
		dataBuf = new double*[num_ch];
		for (int ch = 0; ch < num_ch; ch++) {
			dataBuf[ch] = new double[sFFTLength];
		}

		// memory allocation
		int n = sFFTLength * 2;
		int* ip = new int[2 + (int)(ceil(sqrt(n)))];
		double* w = new double[sFFTLength];
		double** src_fft_data = new double*[num_ch];
		double** out_fft_data = new double*[num_ch];
		double** temp_out_data = new double*[num_ch];
		double** out_data = new double*[num_ch];
		for (int ch = 0; ch < num_ch; ch++) {
			src_fft_data[ch] = new double[n];
			out_fft_data[ch] = new double[n];
			temp_out_data[ch] = new double[sFFTLength / 2];
			out_data[ch] = new double[wavSamples + sFFTLength / 2];
		}

		for (int ch = 0; ch < num_ch; ch++) {
			for (int i = 0; i < sFFTLength / 2; i++) {
				temp_out_data[ch][i] = 0;
			}
		}
		double fft_denom = sFFTLength * sqrt(sFFTLength);
		double aPeak = 1.0;

		int aCurPosition = 0;
		while (aCurPosition < wavSamples) {
			int aRemainSize = wavSamples - aCurPosition;
			int aReadDataSize = (aRemainSize >= sFFTLength / 2) ? sFFTLength / 2 : aRemainSize;
			bool aIsFullfilled = (aRemainSize >= sFFTLength / 2) ? true : false;

			for (int i = 0; i < aReadDataSize; i++) {
				for (int ch = 0; ch < num_ch; ch++) {
					dataBuf[ch][i] = ReadWaveData(FP_SrcWaveFile, &formatChunkBuf);
				}
			}

			for (int ch = 0; ch < num_ch; ch++) {
				for (int i = 0; i < aReadDataSize; i++) {
					REAL(src_fft_data[ch], i) = dataBuf[ch][i];
					IMAG(src_fft_data[ch], i) = 0.0;
				}
				for (int i = aReadDataSize; i < sFFTLength; i++) {
					REAL(src_fft_data[ch], i) = IMAG(src_fft_data[ch], i) = 0.0;
				}
			}

			for (int ch = 0; ch < num_ch; ch++) {
				ip[0] = 0;
				cdft(n, -1, src_fft_data[ch], ip, w);
			}

			for (int i = 0; i < 64; i++) {
				for (int ch = 0; ch < num_ch; ch++) {
					std::complex<double> aSrc(REAL(src_fft_data[ch], i), IMAG(src_fft_data[ch], i));
					double aMag = abs(aSrc);
					double aArg = arg(aSrc);
					printf("%d mag %g arg %g", i, aMag, aArg * 360 / (2 * M_PI));
				}
				printf("\n");
			}

			for (int ch = 0; ch < num_ch; ch++) {
				int aIrChannel = ch % sIR_Channel;
				for (int i = 0; i < sFFTLength; i++) {
					std::complex<double> aIR(REAL(ir_fft_data[aIrChannel], i), IMAG(ir_fft_data[aIrChannel], i));
					std::complex<double> aSrc(REAL(src_fft_data[ch], i), IMAG(src_fft_data[ch], i));
#if 0
					std::complex<double> aResult = aSrc * aIR;	// convolution
#else
					std::complex<double> aResult = aSrc / aIR;	// deconvolution
					if (i == 0) { aResult *= abs(aIR); }
#endif
					REAL(out_fft_data[ch], i) = aResult.real();
					IMAG(out_fft_data[ch], i) = aResult.imag();
				}

				ip[0] = 0;
				cdft(n, 1, out_fft_data[ch], ip, w);

				for (int i = 0; i < sFFTLength / 2; i++) {
					double aData = temp_out_data[ch][i] + REAL(out_fft_data[ch], i) / sFFTLength;
					out_data[ch][aCurPosition + i] = aData;
					if ((aPeak < fabs(aData)) && aIsFullfilled) {
						aPeak = fabs(aData);
					}
				}
			}

			for (int ch = 0; ch < num_ch; ch++) {
				for (int i = 0; i < sFFTLength / 2; i++) {
					temp_out_data[ch][i] = REAL(out_fft_data[ch], sFFTLength / 2 + i) / sFFTLength;
				}
			}

			aCurPosition += sFFTLength / 2;
		}

		for (int i = 0; i < sFFTLength / 2; i++) {
			for (int ch = 0; ch < num_ch; ch++) {
				double aData = temp_out_data[ch][i];
				out_data[ch][wavSamples + i] = aData;
			}
		}

		printf("detected peak: %g\n", aPeak);

		for (int i = 0; i < wavSamples + sFFTLength / 2; i++) {
			for (int ch = 0; ch < num_ch; ch++) {
				WriteWaveData(FP_OutWaveFile, &formatChunkBuf, out_data[ch][i] / aPeak);
			}
		}

		for (int ch = 0; ch < num_ch; ch++) {
			delete[] dataBuf[ch];
			delete[] src_fft_data[ch];
		}
		delete[] dataBuf;
		delete[] src_fft_data;
		delete[] ip;
		delete[] w;

		CopyFile(FP_SrcWaveFile, FP_OutWaveFile, -1);

		MaintainWavHeader(FP_OutWaveFile, (wavSamples + sFFTLength / 2) * formatChunkBuf.blockSize, dataStartPos);
	}
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

