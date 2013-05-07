#include <stdio.h>
#include <string.h>
#include <math.h>

typedef signed long	CoefData;

#define         PI      3.14159265358979
#define         M6DB    0.5011872336
#define         FSAMP   32000.0

int	mFreq = 1629;
double	mLevelSent = -8.0;
double	mQSent = 9.51366;
int mType = 0;


//////////////////////////////////////////////////////////////////////////////
// TN_EQ class
//////////////////////////////////////////////////////////////////////////////
char* EqEncodeCoefData(double dCoefData)
{
    static char tmpStr[20];
    CoefData tmpCoefData;
    if (dCoefData > 16.0) {
	return "0x07FFFFFF";
    } else if (dCoefData <= -16.0) {
        return "0xF8000000";
    } else {
	tmpCoefData = (CoefData)(dCoefData * 0x800000L);
	sprintf(tmpStr, "0x%08x", tmpCoefData);
            return tmpStr;
    }
}

void EqFaLowPassCal(const char** StepName)
{
    float th, t0, w0;
    float xa, xb, xc;

    th = PI * mFreq / FSAMP;
    t0 = 1.0 / FSAMP;
    w0 = 2.0 * FSAMP * tan(th);

    printf("w0 = %f, t0 = %f, th = %f\n", w0, t0, th);

    xa = w0 * t0 / (2.0 + w0 * t0);
    xb = xa;
    xc = (2.0 - w0 * t0) / (2.0 + w0 * t0);

    printf("	{%s,	%s,	%s}, /* %f */ /* ceq FA 00 13 %d %f 1 %f */\n", 
	   StepName[0], StepName[1], EqEncodeCoefData(xa), xa, mFreq, mLevelSent, FSAMP);
    printf("	{%s,	%s,	%s}, /* %f */\n", StepName[2], StepName[3], EqEncodeCoefData(xb), xb);
    printf("	{%s,	%s,	%s}, /* %f */\n", StepName[4], StepName[5], EqEncodeCoefData(xc), xc);
    printf("\n");
}

void EqFaHighPassCal(const char** StepName)
{
    float th, t0, w0;
    float xa, xb, xc;

    th = PI * mFreq / FSAMP;
    t0 = 1.0 / FSAMP;
    w0 = 2.0 * FSAMP * tan(th);

    printf("w0 = %f, t0 = %f, th = %f\n", w0, t0, th);

    xa = 2.0 / (2.0 + w0 * t0);
    xb = -xa;
    xc = (2.0 - w0 * t0) / (2.0 + w0 * t0);

    printf("	{%s,	%s,	%s}, /* %f */ /* ceq FA 00 14 %d %f 1 %f */\n", 
	   StepName[0], StepName[1], EqEncodeCoefData(xa), xa, mFreq, mLevelSent, FSAMP);
    printf("	{%s,	%s,	%s}, /* %f */\n", StepName[2], StepName[3], EqEncodeCoefData(xb), xb);
    printf("	{%s,	%s,	%s}, /* %f */\n", StepName[4], StepName[5], EqEncodeCoefData(xc), xc);
    printf("\n");
}

void EqFaLowShelvingCal(const char** StepName)
{
    float th, dCos, dSin, dG;
    float k0, d0, d1, d2, xa, xb, xc;

    th = PI * mFreq / FSAMP;
    dCos = cos(2.0 * th);
    dSin = sin(2.0 * th);
    dG = pow(10.0, fabs(mLevelSent) / 20.0);

    if (dCos == 0.0)
        xc = 0.0;
    else {
        k0 = 2.0 * M6DB - 1.0;
        d0 = fabs(dSin) * sqrt(1.0 - k0 * k0);
        d1 = k0 * dCos - 1.0;
        d2 = k0 + dCos;
        xc = -1.0 * (d1 + d0) / d2;
        if (fabs(xc) >= 1.0)
            xc = -1.0 * (d1 - d0) / d2;
    }
    xa = 0.5 * ((1.0 - xc) / dG + (1.0 + xc));
    xb = 0.5 * ((1.0 - xc) / dG - (1.0 + xc));

    printf("th = %f, k0 = %f, d0 = %f, d1 = %f, d2 = %f, xa = %f, xb = %f, xc = %f\n", th, k0, d0, d1, d2, xa, xb, xc);

    if (mLevelSent >= 0.0) {
	    printf("	{%s,	%s,	%s}, /* %f */ /* ceq FA 00 11 %d %f 1 %f */\n", 
		   StepName[0], StepName[1], EqEncodeCoefData(1.0 / xa), 1.0 / xa, mFreq, mLevelSent, FSAMP);
	    printf("	{%s,	%s,	%s}, /* %f */\n", StepName[2], StepName[3], EqEncodeCoefData(-xc / xa), -xc / xa);
	    printf("	{%s,	%s,	%s}, /* %f */\n", StepName[4], StepName[5], EqEncodeCoefData(-xb / xa), -xb / xa);
	    printf("\n");
    } else {
	    printf("	{%s,	%s,	%s}, /* %f */ /* ceq FA 00 11 %d %f 1 %f */\n", 
		   StepName[0], StepName[1], EqEncodeCoefData(xa), xa, mFreq, mLevelSent, FSAMP);
	    printf("	{%s,	%s,	%s}, /* %f */\n", StepName[2], StepName[3], EqEncodeCoefData(xb), xb);
	    printf("	{%s,	%s,	%s}, /* %f */\n", StepName[4], StepName[5], EqEncodeCoefData(xc), xc);
	    printf("\n");
    }
}

void EqFaHiShelvingCal(const char** StepName)
{
    float th, dCos, dSin, dG;
    float k0, d0, d1, d2, xa, xb, xc;

    th = PI * mFreq / FSAMP;

    dCos = cos(2.0 * th);
    dSin = sin(2.0 * th);
    dG = pow(10.0, fabs(mLevelSent) / 20.0);

    if (dCos == 0.0)
        xc = 0.0;
    else {
        k0 = 2.0 * M6DB - 1.0;
        d0 = fabs(dSin) * sqrt(1.0 - k0 * k0);
        d1 = k0 * dCos + 1.0;
        d2 = k0 - dCos;
        xc = -1.0 * (d1 + d0) / d2;
        if (fabs(xc) >= 1.0)
            xc = -1.0 * (d1 - d0) / d2;
    }
    xa = 0.5 * ((1.0 - xc) + (1.0 + xc) / dG);
    xb = 0.5 * ((1.0 - xc) - (1.0 + xc) / dG);

    printf("th = %f, k0 = %f, d0 = %f, d1 = %f, d2 = %f, xa = %f, xb = %f, xc = %f\n", th, k0, d0, d1, d2, xa, xb, xc);

    if (mLevelSent >= 0.0) {
	    printf("	{%s,	%s,	%s}, /* %f */ /* ceq FA 00 12 %d %f 1 %f */\n", 
		   StepName[0], StepName[1], EqEncodeCoefData(1.0 / xa), 1.0 / xa, mFreq, mLevelSent, FSAMP);
	    printf("	{%s,	%s,	%s}, /* %f */\n", StepName[2], StepName[3], EqEncodeCoefData(-xc / xa), -xc / xa);
	    printf("	{%s,	%s,	%s}, /* %f */\n", StepName[4], StepName[5], EqEncodeCoefData(-xb / xa), -xb / xa);
	    printf("\n");
    } else {
	    printf("	{%s,	%s,	%s}, /* %f */ /* ceq FA 00 12 %d %f 1 %f */\n", 
		   StepName[0], StepName[1], EqEncodeCoefData(xa), xa, mFreq, mLevelSent, FSAMP);
	    printf("	{%s,	%s,	%s}, /* %f */\n", StepName[2], StepName[3], EqEncodeCoefData(xb), xb);
	    printf("	{%s,	%s,	%s}, /* %f */\n", StepName[4], StepName[5], EqEncodeCoefData(xc), xc);
	    printf("\n");
    }
}

void EqVePeakingCal(const char** StepName)
{
    double dTh, dTanTh, dG;
    double dDD, dPP, dQQ, dRoot, dC, dSq, dQualt;
    
    dQualt = mQSent;
    dTh = mFreq / FSAMP * PI;
    dTanTh = tan(dTh);
    dG = pow(10.0, fabs(mLevelSent) / 20.0);
    dQQ = (dTh * dQualt) / dTanTh;
    dC = 1.0;
    dPP = 0.8 * pow(10.0, 12.0 / 20.0);

    if (mLevelSent >= 0) {
        if (dQualt < 1.0) {
            dC = dQQ;
        }
    } else {
        dPP = dPP / dG;
        dQQ = dQQ / dG;
        if ((dQualt / dG) < 1.0) {
            dC = dQQ;
        }
    }

    dSq = 2.0 - (2.0 * cos(2.0 * dTh));
    dRoot = sqrt(dSq);
    dDD = 1.0 / (dQQ * (dTanTh * dTanTh) + dTanTh + dQQ);

    printf("	{%s,	%s,	%s}, /* %f */ /* ceq VE 00 26 %d %f %f %f */\n",
	   StepName[0], StepName[1], EqEncodeCoefData((dPP / dRoot) * (dTanTh * dDD) * dC),
	   (dPP / dRoot) * (dTanTh * dDD) * dC, mFreq, mLevelSent, mQSent, FSAMP);
    printf("	{%s,	%s,	%s}, /* %f */\n", StepName[2], StepName[3], 
	   EqEncodeCoefData((dPP / dRoot) * (dTanTh * dDD) * dC), (dPP / dRoot) * (dTanTh * dDD) * dC);
    printf("	{%s,	%s,	%s}, /* %f */\n", StepName[4], StepName[5], 
	   EqEncodeCoefData(dRoot), dRoot);
    printf("	{%s,	%s,	%s}, /* %f */\n", StepName[6], StepName[7], 
	   EqEncodeCoefData((-2.0 * dTanTh * dDD) / dRoot), (-2.0 * dTanTh * dDD) / dRoot);
    printf("	{%s,	%s,	%s}, /* %f */\n", StepName[8], StepName[9], 
	   EqEncodeCoefData(dRoot), dRoot);
    printf("	{%s,	%s,	%s}, /* %f */\n", StepName[12], StepName[13], 
	   EqEncodeCoefData((pow(10.0, (mLevelSent / 20.0)) - 1.0) / (dPP * dC)), (pow(10.0, (mLevelSent / 20.0)) - 1.0) / (dPP * dC));
    printf("\n");
}


void saveAllDataAsCoef(int mType,  const char** StepName)
{
    
    	switch(mType){
                case 0x00:
                        EqFaLowShelvingCal(StepName);
                        break;
                case 0x01:
                        EqFaHiShelvingCal(StepName);
                        break;
                case 0x02:
                        EqFaLowPassCal(StepName);
                        break;
                case 0x03:
                        EqFaHighPassCal(StepName);
                        break;
                case 0x04:
                        EqVePeakingCal(StepName);
                        break;
             default:
                        break;
                }
}
// /////////////////////////////////////////////
void main(void)
{
	static const char* oku_preEQ_FA[] = {
		"COE_PF000_0", "COE_PF100_0",
		"COE_PF001_0", "COE_PF101_0",
		"COE_PF002_0", "COE_PF102_0",
		
		"COE_PF010_0", "COE_PF110_0",
		"COE_PF011_0", "COE_PF111_0",
		"COE_PF012_0", "COE_PF112_0",
	};
	static const char* oku_preEQ_VE[] = {
		"COE_EQ000_0", "COE_EQ100_0",
		"COE_EQ001_0", "COE_EQ101_0",
		"COE_EQ002_0", "COE_EQ102_0",
		"COE_EQ003_0", "COE_EQ103_0",
		"COE_EQ004_0", "COE_EQ104_0",
		"0xFFFFFFFF", "0xFFFFFFFF",
		"COE_EQ006_0", "COE_EQ106_0",
		
		"COE_EQ010_0", "COE_EQ110_0",
		"COE_EQ011_0", "COE_EQ111_0",
		"COE_EQ012_0", "COE_EQ112_0",
		"COE_EQ013_0", "COE_EQ113_0",
		"COE_EQ014_0", "COE_EQ114_0",
		"0xFFFFFFFF", "0xFFFFFFFF",
		"COE_EQ016_0", "COE_EQ116_0",
	};

	printf("static const SDspStCoefDat oku_preEQ_FA[] = {\n");

	saveAllDataAsCoef( 0, &oku_preEQ_FA[0] );
	saveAllDataAsCoef( 1, &oku_preEQ_FA[6] );

	printf("\n");
					
	printf("static const SDspStCoefDat oku_preEQ_VE[] = {\n");

	saveAllDataAsCoef( 4, &oku_preEQ_VE[0] );
	saveAllDataAsCoef( 4, &oku_preEQ_VE[14] );

	printf("\n");


}

