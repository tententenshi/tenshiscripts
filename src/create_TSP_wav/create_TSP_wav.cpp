#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex>
#include "../wav.h"


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void Usage(const char* command)
{
	fprintf(stderr, "make TSP (Time Stretched Pulse)\n");
	fprintf(stderr, "%s Length_In_Samples [Pink (to generate Pink-TSP)] [m_parameter(less than Length_In_Samples / 2)] output filename.wav\n\n", command);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void Process(FILE *fp, const SFormatChunk& formatChunk, int wavSamples, int m_parameter, bool isPink);

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	if ((argc != 3) && (argc != 4)) {
		Usage(argv[0]);
		exit(1);
	}

	argv++;
	int wavSamples = atoi(*argv++);
	bool isPink = false;
	if (strcmp(*argv, "Pink") == 0) {
		isPink = true;
		argv++;
	}
	int m_parameter = wavSamples * 3 / 8;
	if (atoi(*argv) > 0) {
		m_parameter = atoi(*argv++);
	}
	char* outfile = *argv++;

	FILE *fp;

	if ((fp = fopen(outfile, "wb")) == NULL) {
		printf("Cannot open %s\n", outfile);
		exit(1);
	}

	SFormatChunk formatChunkBuf;
	int32_t dataStartPos = CreateWavHeader(fp, &formatChunkBuf, 1, 44100, 32, 3);	// 32bit float, 44.1kHz

	wavSamples = (int)pow(2, ceil(log(wavSamples)/log(2)));

	if (wavSamples > 0) {
		Process(fp, formatChunkBuf, wavSamples, m_parameter, isPink);
	}

	MaintainWavHeader(fp, wavSamples * formatChunkBuf.blockSize, dataStartPos);

	fclose(fp);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
/* accessor for real and imaginary part of complex array */
#define REAL(z,i) ((z)[2*(i)])
#define IMAG(z,i) ((z)[2*(i)+1])

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void InverseFFT(int theSamples, double* fft_buf)
{
	// memory allocation
	int n = theSamples * 2;
	int* ip = new int[2 + (int)(ceil(sqrt(n)))];
	double* w = new double[theSamples];
	ip[0] = 0;
	extern void cdft(int n, int isgn, double *a, int *ip, double *w);
	cdft(n, 1, fft_buf, ip, w);
	delete[] ip;
	delete[] w;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
inline double GetTSP_Arg(int wavSamples, int m_parameter, int i)
{
	double theta = (double)i / wavSamples;
	double omega = -4 * m_parameter * M_PI * theta * theta;
	return omega;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
inline double GetPinkTSP_Arg(int wavSamples, int m_parameter, int i)
{
	double a = -4 * m_parameter * M_PI / (wavSamples * log(wavSamples / 2));
	double omega = a * i * log(i);
	return omega;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void Process(FILE *fp, const SFormatChunk& formatChunk, int wavSamples, int m_parameter, bool isPink)
{
	int FSAMP = formatChunk.fsamp;
	int num_ch = formatChunk.channel;

	double* fft_buf = new double[wavSamples * 2];

	{
		REAL(fft_buf, 0) = 1;
		IMAG(fft_buf, 0) = 0;
	}
	for (int i = 1; i < wavSamples / 2; i++) {
		double omega, aMag;
		if (isPink) {
			omega = GetPinkTSP_Arg(wavSamples, m_parameter, i);
			aMag = 1 / sqrt(i);
		} else {
			omega = GetTSP_Arg(wavSamples, m_parameter, i);
			aMag = 1;
		}
		REAL(fft_buf, i) = aMag * cos(omega);
		IMAG(fft_buf, i) = aMag * sin(omega);
	}
	for (int i = wavSamples / 2; i < wavSamples; i++) {
		int k = wavSamples - i;
		double omega, aMag;
		if (isPink) {
			omega = GetPinkTSP_Arg(wavSamples, m_parameter, k);
			aMag = 1 / sqrt(k);
		} else {
			omega = GetTSP_Arg(wavSamples, m_parameter, k);
			aMag = 1;
		}
		REAL(fft_buf, i) = aMag * cos(-omega);
		IMAG(fft_buf, i) = aMag * sin(-omega);
	}

	InverseFFT(wavSamples, fft_buf);

	double aPeak = 0.001;
	for (int i = 0; i < wavSamples; i++) {
		double val = REAL(fft_buf, i);
		if (aPeak < fabs(val)) { aPeak = fabs(val); }
	}
	double aMagnitude = 1 / aPeak;

	for (int i = 0; i < wavSamples; i++) {
		for (int ch = 0; ch < num_ch; ch++) {
			int i_offset;
			if (isPink) {
				// http://web.ck.miyakoda.roland.co.jp/piano/software/dsp/TimeStretchedPulse/IRseminor2016.pdf
				i_offset = (i + 2 * m_parameter + (int)(2 * m_parameter / log(wavSamples / 2))) % wavSamples;
			} else {
				i_offset = (i + wavSamples / 2 + m_parameter) % wavSamples;
			}
			double val = REAL(fft_buf, i_offset) * aMagnitude;
			WriteWaveData(fp, &formatChunk, val);
		}
	}

	delete[] fft_buf;
}

