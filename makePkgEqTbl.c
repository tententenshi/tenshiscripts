#include <math.h>
#include <stdio.h>

typedef	long CoefData;

#define	FSAMP	44100.0
#define	PI	3.1415926

#define	NumOfElem(array)	(int)(sizeof(array)/sizeof(array[0]))
/*
 *--------------------------------------------------------------
 * EqEncodeCoefData
 *--------------------------------------------------------------
 *
 *	prog :	tnisimu
 *	func :	Encode coef from double to CoefData
 *	entry:	double		dCoefData
 *	retrn:	CoefData	CoefData 	 //   ssss-ssss sxxx-xxxx xxxx-xxxx xxxx-xxxx (32bit)
 *					 	 //     Decimal point is bitween bit23 and bit22.
 *	remks:	----
 *
 *--------------------------------------------------------------
 */
CoefData EqEncodeCoefData(double dCoefData)
{
    if (dCoefData > 16.0) {
	return 0x07FFFFFF;
    } else if (dCoefData <= -16.0) {
	return 0xF8000000;
    } else {
	    return (CoefData)(dCoefData * 0x00800000);
    }
}
double Ans0, Ans2, Ans3, Ans4, Ans6;
CoefData aCoef[7];

void EqVePeakingCal(short sGain, short sFreq, short sQualt, CoefData *sEqFaCoefDat)
{
    double dQualt = sQualt / 100.0;
    double dTh = (double)sFreq / FSAMP * PI;
    double dTanTh = tan(dTh);				// prewarping
    double dQQ = (dTh * dQualt) / dTanTh;		// prewarping
    double dG = pow(10.0, fabs((double)sGain) / 200);
    double dPP = 0.8 * pow(10.0, 12.0 / 20.0);

    double d1 = 1.0;
    double a1 = dTanTh / dQQ;

    double dRoot, k0;

    if (sGain >= 0.0) {
	if (dQualt < 1.0) {
	    d1 = dQQ;
	}
    } else {
    	a1 = a1 * dG;
	if((dQualt / dG) < 1.0) {
	    d1 = dQQ / dG;
	}
    }

    dRoot = sqrt(2.0 - (2.0 * cos(2.0 * dTh)));
    k0 = (dTanTh * dTanTh) + a1 + 1.0;

    Ans0 = dPP * a1 / (dRoot * k0) * d1;
    Ans2 = dRoot;
    Ans3 = (-2.0 * a1) / (dRoot * k0);
    Ans4 = 4.0 * (dTanTh * dTanTh) / (dRoot * k0);
    Ans6 = (pow(10.0, (sGain / 200.0)) - 1.0) / (dPP * d1);

    sEqFaCoefDat[0] = sEqFaCoefDat[1] = EqEncodeCoefData(Ans0);
    sEqFaCoefDat[2] = EqEncodeCoefData(Ans2);
    sEqFaCoefDat[3] = EqEncodeCoefData(Ans3);
    sEqFaCoefDat[4] = EqEncodeCoefData(Ans4);
    sEqFaCoefDat[5] = EqEncodeCoefData(-1.0);	// fixed
    sEqFaCoefDat[6] = EqEncodeCoefData(Ans6);
}

void printResult(void)
{
	printf("coef0 -> 0x%08x(%12.8f)\n", aCoef[0], Ans0);
	printf("coef1 -> 0x%08x(%12.8f)\n", aCoef[1], Ans0);
	printf("coef2 -> 0x%08x(%12.8f)\n", aCoef[2], Ans2);
	printf("coef3 -> 0x%08x(%12.8f)\n", aCoef[3], Ans3);
	printf("coef4 -> 0x%08x(%12.8f)\n", aCoef[4], Ans4);
	printf("coef5 -> 0x%08x(%12.8f)\n", aCoef[5], -1.0);
	printf("coef6 -> 0x%08x(%12.8f)\n", aCoef[6], Ans6);
	printf("\n");
}

void FormatResult(short sGain, short sFreq, short sQualt)
{
	printf("\t\t\t{0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x},\t// %5dHz %5.1fdB Q%4.1f\n",
	       aCoef[0], aCoef[2], aCoef[3], aCoef[4], aCoef[6], sFreq, sGain / 10.0, sQualt / 100.0);
}

static double min[7] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
static double max[7];
typedef struct {
	short	gain;
	short	freq;
	short	q;
} SCond;
SCond	minCond[7], maxCond[7];
void StoreMinMax(short sGain, short sFreq, short sQualt)
{
	if (fabs(min[0]) > fabs(Ans0)) {min[0] = Ans0; minCond[0].gain = sGain; minCond[0].freq = sFreq; minCond[0].q = sQualt; }
	if (fabs(min[2]) > fabs(Ans2)) {min[2] = Ans2; minCond[2].gain = sGain; minCond[2].freq = sFreq; minCond[2].q = sQualt; }
	if (fabs(min[3]) > fabs(Ans3)) {min[3] = Ans3; minCond[3].gain = sGain; minCond[3].freq = sFreq; minCond[3].q = sQualt; }
	if (fabs(min[4]) > fabs(Ans4)) {min[4] = Ans4; minCond[4].gain = sGain; minCond[4].freq = sFreq; minCond[4].q = sQualt; }
	if (fabs(min[6]) > fabs(Ans6)) {min[6] = Ans6; minCond[6].gain = sGain; minCond[6].freq = sFreq; minCond[6].q = sQualt; }

	if (fabs(max[0]) < fabs(Ans0)) {max[0] = Ans0; maxCond[0].gain = sGain; maxCond[0].freq = sFreq; maxCond[0].q = sQualt; }
	if (fabs(max[2]) < fabs(Ans2)) {max[2] = Ans2; maxCond[2].gain = sGain; maxCond[2].freq = sFreq; maxCond[2].q = sQualt; }
	if (fabs(max[3]) < fabs(Ans3)) {max[3] = Ans3; maxCond[3].gain = sGain; maxCond[3].freq = sFreq; maxCond[3].q = sQualt; }
	if (fabs(max[4]) < fabs(Ans4)) {max[4] = Ans4; maxCond[4].gain = sGain; maxCond[4].freq = sFreq; maxCond[4].q = sQualt; }
	if (fabs(max[6]) < fabs(Ans6)) {max[6] = Ans6; maxCond[6].gain = sGain; maxCond[6].freq = sFreq; maxCond[6].q = sQualt; }
}

void PrintMinMax(int theC)
{
	printf("coef%d -> min 0x%08x(%12.8f) -> %5dHz %5.1fdB Q%4.1f, max 0x%08x(%12.8f) -> %5dHz %5.1fdB Q%4.1f\n",
		theC,
		EqEncodeCoefData(min[theC]), min[theC], minCond[theC].freq, minCond[theC].gain / 10.0, minCond[theC].q / 100.0,
		EqEncodeCoefData(max[theC]), max[theC], maxCond[theC].freq, maxCond[theC].gain / 10.0, maxCond[theC].q / 100.0);
}

static const int FreqTbl[] = {
	                 16,    20,    25,    31,    40,    50,    63,    80,
	  100,   125,   160,   200,   250,   315,   400,   500,   630,   800,
	 1000,  1250,  1600,  2000,  2500,  3150,  4000,  5000,  6300,  8000,
	10000, 12500, 16000,
};
static const int GainTbl[] = {
	-120, -118, -116, -114, -112, -110, -108, -106, -104, -102,
	-100,  -98,  -96,  -94,  -92,  -90,  -88,  -86,  -84,  -82,
	 -80,  -78,  -76,  -74,  -72,  -70,  -68,  -66,  -64,  -62,
	 -60,  -58,  -56,  -54,  -52,  -50,  -48,  -46,  -44,  -42,
	 -40,  -38,  -36,  -34,  -32,  -30,  -28,  -26,  -24,  -22,
	 -20,  -18,  -16,  -14,  -12,  -10,   -8,   -6,   -4,   -2,
	   0,
	   2,    4,    6,    8,   10,   12,   14,   16,   18,   20,
	  22,   24,   26,   28,   30,   32,   34,   36,   38,   40,
	  42,   44,   46,   48,   50,   52,   54,   56,   58,   50,
	  62,   64,   66,   68,   70,   72,   74,   76,   78,   70,
	  82,   84,   86,   88,   90,   92,   94,   96,   98,  100,
	 102,  104,  106,  108,  110,  112,  114,  116,  118,  120,
};
static const int GainNegTbl[] = {
	-120, -118, -116, -114, -112, -110, -108, -106, -104, -102,
	-100,  -98,  -96,  -94,  -92,  -90,  -88,  -86,  -84,  -82,
	 -80,  -78,  -76,  -74,  -72,  -70,  -68,  -66,  -64,  -62,
	 -60,  -58,  -56,  -54,  -52,  -50,  -48,  -46,  -44,  -42,
	 -40,  -38,  -36,  -34,  -32,  -30,  -28,  -26,  -24,  -22,
	 -20,  -18,  -16,  -14,  -12,  -10,   -8,   -6,   -4,   -2,
	   0,
};
static const int GainPosTbl[] = {
	   2,    4,    6,    8,   10,   12,   14,   16,   18,   20,
	  22,   24,   26,   28,   30,   32,   34,   36,   38,   40,
	  42,   44,   46,   48,   50,   52,   54,   56,   58,   50,
	  62,   64,   66,   68,   70,   72,   74,   76,   78,   70,
	  82,   84,   86,   88,   90,   92,   94,   96,   98,  100,
	 102,  104,  106,  108,  110,  112,  114,  116,  118,  120,
};

static const int QTbl[] = {
	50, 100, 200, 400, 800,
};

int main(void)
{
	int i, j, k;
#if 0
	EqVePeakingCal(120, 16000, 800, aCoef);
	printResult();

	EqVePeakingCal(120, 16000, 50, aCoef);
	printResult();

	EqVePeakingCal(-120, 16000, 800, aCoef);
	printResult();

	EqVePeakingCal(-120, 16000, 50, aCoef);
	printResult();

	EqVePeakingCal(120, 10, 800, aCoef);
	printResult();

	EqVePeakingCal(120, 10, 50, aCoef);
	printResult();

	EqVePeakingCal(-120, 10, 800, aCoef);
	printResult();

	EqVePeakingCal(-120, 10, 50, aCoef);
	printResult();
#endif
#if 1
	for (i = 0; i < NumOfElem(FreqTbl); i++) {
		for (j = 0; j < NumOfElem(QTbl); j++) {
			for (k = 0; k < NumOfElem(GainTbl); k++) {
				EqVePeakingCal(GainTbl[k], FreqTbl[i], QTbl[j], aCoef);
				StoreMinMax(GainTbl[k], FreqTbl[i], QTbl[j]);
			}
		}
	}

	for (i = 0; i < NumOfElem(FreqTbl); i++) {
		printf("\t{\n");
		for (j = 0; j < NumOfElem(QTbl); j++) {
			printf("\t\t{\n");
			for (k = 0; k < NumOfElem(GainNegTbl); k++) {
				EqVePeakingCal(GainNegTbl[k], FreqTbl[i], QTbl[j], aCoef);
				FormatResult(GainNegTbl[k], FreqTbl[i], QTbl[j]);
			}
			printf("\t\t},\n");
		}
		printf("\t},\n");
	}

	for (j = 1; j < 2; j++) {
		for (i = 0; i < 1; i++) {
			for (k = 0; k < NumOfElem(GainPosTbl); k++) {
				EqVePeakingCal(GainPosTbl[k], FreqTbl[i], QTbl[j], aCoef);
				printf("\t0x%08x,\t// %5.1fdB Q >= 1\n", aCoef[6], GainPosTbl[k] / 10.0);
			}
			printf("\n");
		}
		printf("\n");
	}

	for (j = 0; QTbl[j] < 100; j++) {
		printf("\t{\n");
		for (i = 0; i < NumOfElem(FreqTbl); i++) {
			printf("\t\t{\n");
			for (k = 0; k < NumOfElem(GainPosTbl); k++) {
				EqVePeakingCal(GainPosTbl[k], FreqTbl[i], QTbl[j], aCoef);
				printf("\t\t\t0x%08x,\t// %5dHz %5.1fdB Q%4.1f\n",
					aCoef[6], FreqTbl[i], GainPosTbl[k] / 10.0, QTbl[j] / 100.0);
			}
			printf("\t\t},\n");
		}
		printf("\t},\n");
	}

	PrintMinMax(0);
	PrintMinMax(2);
	PrintMinMax(3);
	PrintMinMax(4);
	PrintMinMax(6);
#endif
}


