//----------------------------------------------------------------------------
// ceq  (type,calcmode,filtermode,freq,level,quality)
//
//-----------------------------------------------------------------------------
#include 	<stdio.h>
#include 	<math.h>

#define         PI      3.14159265358979
#define         M6DB    0.5011872336
#define		FSAMP	32000.0

#define	ROUND(x)	(((x) < 0) ? -0.5 : 0.5 )

double ceq[10][2];

void vetest1(short sGain, short sFreq, short sQualt)
{
    float dTh, dTanTh, dG;
    float dDD, dPP, dQQ, dRoot, dC, dSq, dQualt;

    dQualt = sQualt / 100.0;
    dTh = sFreq / FSAMP * PI;
    dTanTh = tan(dTh);
    dG = pow(10.0, fabs(sGain) / 200);
    dQQ = (dTh * dQualt) / dTanTh;
    dC = 1.0;
    dPP = 0.8 * pow(10.0, 12.0 / 20.0);

    if (sGain >= 0) {
	if (dQualt < 1.0) {
	    dC = dQQ;
	}
    } else {			/* different between below arithmetic */
	dPP = dPP / dG;
	dQQ = dQQ / dG;
	if ((dQualt / dG) < 1.0) {
		dC = dQQ;
	}
    }

    dSq = 2.0 - (2.0 * cos(2.0 * dTh));
    dRoot = sqrt(dSq);
    dDD = 1.0 / (dQQ * (dTanTh * dTanTh) + dTanTh + dQQ);

    ceq[0][0] = ceq[1][0] = ((dPP / dRoot) * (dTanTh * dDD) * dC);
    ceq[2][0] = (dRoot);
    ceq[3][0] = ((-2.0 * dTanTh * dDD) / dRoot);
    ceq[4][0] = -1.0 * ((-4.0 * dQQ * dTanTh * dTanTh * dDD) / dSq ) * dRoot;
    ceq[5][0] = -1.0;
    ceq[6][0] = ((pow(10.0, (sGain / 200.0)) - 1.0) / (dPP * dC));

}

void vetest2(short sGain, short sFreq, short sQualt)
{
	double d1,d7,mg; 
	double qualt, t0, t2, n4, mm, th, w0, w2, g0, xq, gx, xp;
	double a1, b1, k0, ya, xa, yb;

	mg = 0.8;
	t0 = 1.0 / FSAMP;
	t2 = t0 * t0;
	n4 = pow(10.0, 12.0 / 20.0);

	qualt = sQualt / 100.0;
	th = sFreq / FSAMP * PI;
	w0 = 2.0 * FSAMP * tan(th);
	w2 = w0 * w0;
	mm = sqrt(2.0 - 2.0 * cos(2.0 * th));
	g0 = pow(10.0, fabs(sGain) / 200.0);
	xq = qualt * PI * sFreq / (FSAMP * tan(th));
	gx = pow(10.0, sGain / 200.0);
	xp = (gx - 1.0) / n4;


	if(sGain >= 0.0 || sGain < -120.0) {
		a1 = b1 = w0 / xq; 
	} else { 				/* different between above arithmetic */
		a1 = b1 = g0 * w0 / xq;
	}

	k0 = t2 * w2 + 2.0 * t0 * b1 + 4.0;
	ya = (8.0 - 2.0 * t2 * w2)           / k0;
	xa = 2.0 * t0 * a1                   / k0;
	yb = (2.0 * t0 * b1 - 4.0 - t2 * w2) / k0;

	if (sGain >= 0.0 || sGain < -120.0) {
		if(qualt >= 1.0) {
			d1 = 1.0;
			d7 = 1.0;
		} else {
			d1 = xq;
			d7 = 1.0 / xq;
		}
	} else {				/* different between above arithmetic */
		if((qualt / g0) >= 1.0){
			d1 = 1.0;
			d7 = 1.0;
		} else {
			d1 = xq / g0;
			d7 = g0 / xq;
		}
	}

	ceq[0][1] = n4 * xa * mg * d1 / mm;
	ceq[1][1] = ceq[0][1];   
	ceq[2][1] = mm;
	ceq[3][1] = (-yb - 1.0) / mm;
	ceq[4][1] = -1.0 * (ya + yb - 1.0) / mm;
	ceq[5][1] = -1.0;
	ceq[6][1] = xp / mg * d7;

}

void main(int argc,char *argv[])
{
	int i;
	double	freq,level,q;

	if(argc != 4){
		exit(0);
	}
	sscanf(argv[1],"%lf",&freq);
	sscanf(argv[2],"%lf",&level);
	sscanf(argv[3],"%lf",&q);

	vetest1(level * 10, freq, q * 100);
	vetest2(level * 10, freq, q * 100);

	for(i=0; i<10; i++){

		printf("ceq[%d}[0]=%12.9lf \tceq[%d}[1]=%12.9lf \n",i,ceq[i][0], i, ceq[i][1]);
	}
}	

