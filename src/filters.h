/*==============================================================
 *
 * filters.h		filter bank
 *
 *==============================================================
 */
#ifndef _filters_h_
#define _filters_h_

#include <cmath>
#include "stdio.h"

#define	dB2Gain(g)	pow(10.0, (g/20.0))

class CFilterBase {
  public:
	virtual double Push(double theVal) = 0;
	virtual void ShowCoef() = 0;
	virtual void ShowProperty() = 0;
};

class CATT : public CFilterBase {
  private:
	double	FS;
	double 	gain;

  public:
	CATT(double theFS) : FS(theFS) {}
	void SetProperty(double theGain) {
		gain = theGain;
	}
	double Push(double theVal) {
		return theVal * dB2Gain(gain);
	}
	void ShowCoef() {}
	void ShowProperty() {
		printf("att: %g (dB)\n", gain);
	}
};

class CDELAY : public CFilterBase {
  protected:
	double	FS;
	int     mDelay;
	int     mIndex;
	double* mDelayBuf;

  public:
	CDELAY(double theFS) {
		FS = theFS;
		mDelay = 0;
		mIndex = 0;
		mDelayBuf = 0;
	}
	~CDELAY() {
		if (mDelayBuf) { delete [] mDelayBuf; }
	}
	void ShowCoef() {}
	void SetProperty(double theDelay) {
		mDelay = (int)(theDelay * FS + 0.5);
		if (mDelayBuf) { delete [] mDelayBuf; }
		mDelayBuf = new double[mDelay];
		for (int i = 0; i < mDelay; i++) { mDelayBuf[i] = 0; }
		mIndex = 0;
	}
	void ShowProperty() {
		printf("DELAY\n");
		printf(" delay %g (Sec)\n", mDelay / FS);
	}
	double Push(double theVal) {
		int aCurIndex = mIndex;
		mDelayBuf[aCurIndex] = theVal;
		mIndex = (mIndex >= mDelay - 1) ? 0 : mIndex + 1;
		return mDelayBuf[mIndex];
	}
};

class CFA : public CFilterBase {
  protected:
	double	FS;
	double 	a0, a1, b1;
	double	mPrev0, mPrev1;

  public:
	CFA(double theFS) {
		FS = theFS;
		a0 = 1.0;
		a1 = b1 = 0;
		mPrev0 = mPrev1 = 0;
	}
	double Push(double theVal) {
		double aVal = theVal * a0 + mPrev0 * a1 + mPrev1 * b1;
		mPrev0 = theVal;
		mPrev1 = aVal;
		return aVal;
	}
	void ShowCoef() {
		printf("\ta0: %g\n", a0);
		printf("\ta1: %g\n", a1);
		printf("\tb1: %g\n", b1);
	}
};

class CHPF1 : public CFA {
	double mFreq;
  public:
	CHPF1(double theFS) : CFA(theFS) {}
	void SetProperty(double theFreq) {
		mFreq = theFreq;
		double w0 = tan(M_PI * theFreq / FS);
		a0 = 1.0 / (1.0 + w0);
		a1 = -a0;
		b1 = (1.0 - w0) / (1.0 + w0);
	}
	void ShowProperty() {
		printf("HPF1\n");
		printf(" freq %g (Hz)\n", mFreq);
	}
};

class CLPF1 : public CFA {
	double mFreq;
  public:
	CLPF1(double theFS) : CFA(theFS) {}
	void SetProperty(double theFreq) {
		mFreq = theFreq;
		double w0 = tan(M_PI * theFreq / FS);
		a0 = w0 / (1.0 + w0);
		a1 = a0;
		b1 = (1.0 - w0) / (1.0 + w0);
	}
	void ShowProperty() {
		printf("LPF1\n");
		printf(" freq %g (Hz)\n", mFreq);
	}
};

class CHSV1_Trad : public CFA {
	double mFreq;
	double mGain;
  public:
	CHSV1_Trad(double theFS) : CFA(theFS) {}
	void SetProperty(double theFreq, double theGain) {
		mFreq = theFreq; mGain = theGain;
		double w0 = tan(M_PI * theFreq / FS);
		double pole = (theGain < 0) ? w0 : w0 * dB2Gain(theGain);
		double zero = (theGain < 0) ? w0 * dB2Gain(-theGain) : w0;

		a0 = (1.0 + zero) / (1.0 + pole) * dB2Gain(theGain);
		a1 = -(1.0 - zero) / (1.0 + pole) * dB2Gain(theGain);
		b1 = (1.0 - pole) / (1.0 + pole);
	}
	void ShowProperty() {
		printf("HSV1_Trad\n");
		printf(" freq %g (Hz)\n", mFreq);
		printf(" gain %g (dB)\n", mGain);
	}
};

class CLSV1_Trad : public CFA {
	double mFreq;
	double mGain;
  public:
	CLSV1_Trad(double theFS) : CFA(theFS) {}
	void SetProperty(double theFreq, double theGain) {
		mFreq = theFreq; mGain = theGain;
		double w0 = tan(M_PI * theFreq / FS);
		double pole = (theGain < 0) ? w0 : w0 * dB2Gain(-theGain);
		double zero = (theGain < 0) ? w0 * dB2Gain(theGain) : w0;

		a0 = (1.0 + zero) / (1.0 + pole);
		a1 = -(1.0 - zero) / (1.0 + pole);
		b1 = (1.0 - pole) / (1.0 + pole);
	}
	void ShowProperty() {
		printf("LSV1_Trad\n");
		printf(" freq %g (Hz)\n", mFreq);
		printf(" gain %g (dB)\n", mGain);
	}
};

class CHSV1 : public CFA {
	double mFreq;
	double mGain;
  public:
	CHSV1(double theFS) : CFA(theFS) {}
	void SetProperty(double theFreq, double theGain) {
		mFreq = theFreq; mGain = theGain;
		double w0 = tan(M_PI * theFreq / FS);
		double pole = (theGain < 0) ? w0 * dB2Gain(theGain) : w0;
		double zero = (theGain < 0) ? w0 : w0 * dB2Gain(-theGain);

		a0 = (1.0 + zero) / (1.0 + pole) * dB2Gain(theGain);
		a1 = -(1.0 - zero) / (1.0 + pole) * dB2Gain(theGain);
		b1 = (1.0 - pole) / (1.0 + pole);
	}
	void ShowProperty() {
		printf("HSV1\n");
		printf(" freq %g (Hz)\n", mFreq);
		printf(" gain %g (dB)\n", mGain);
	}
};

class CLSV1 : public CFA {
	double mFreq;
	double mGain;
  public:
	CLSV1(double theFS) : CFA(theFS) {}
	void SetProperty(double theFreq, double theGain) {
		mFreq = theFreq; mGain = theGain;
		double w0 = tan(M_PI * theFreq / FS);
		double pole = (theGain < 0) ? w0 * dB2Gain(-theGain) : w0;
		double zero = (theGain < 0) ? w0 : w0 * dB2Gain(theGain);

		a0 = (1.0 + zero) / (1.0 + pole);
		a1 = -(1.0 - zero) / (1.0 + pole);
		b1 = (1.0 - pole) / (1.0 + pole);
	}
	void ShowProperty() {
		printf("LSV1\n");
		printf(" freq %g (Hz)\n", mFreq);
		printf(" gain %g (dB)\n", mGain);
	}
};

class CAPF1 : public CFA {
	double mFreq;
  public:
	CAPF1(double theFS) : CFA(theFS) {}
	void SetProperty(double theFreq) {
		mFreq = theFreq;
		double w0 = tan(M_PI * theFreq / FS);
		a0 = -(1.0 - w0) / (1.0 + w0);
		a1 = 1;
		b1 = (1.0 - w0) / (1.0 + w0);
	}
	void ShowProperty() {
		printf("APF1\n");
		printf(" freq %g (Hz)\n", mFreq);
	}
};


class CEA : public CFilterBase {
  protected:
	double	FS;
	double 	a0, a1, a2, b1, b2;
	double	mPrev00, mPrev01, mPrev10, mPrev11;

  public:
	CEA(double theFS) {
		FS = theFS;
		a0 = 1.0;
		a1 = a2 = b1 = b2 = 0;
		mPrev00 = mPrev01 = mPrev10 = mPrev11 = 0;
	}
	double Push(double theVal) {
		double aVal = theVal * a0 + mPrev00 * a1 + mPrev01 * a2 + mPrev10 * b1 + mPrev11 * b2;
		mPrev01 = mPrev00;
		mPrev11 = mPrev10;
		mPrev00 = theVal;
		mPrev10 = aVal;
		return aVal;
	}
	void ShowCoef() {
		printf("\ta0: %g\n", a0);
		printf("\ta1: %g\n", a1);
		printf("\ta2: %g\n", a2);
		printf("\tb1: %g\n", b1);
		printf("\tb2: %g\n", b2);
	}
};

class CHPF2 : public CEA {
	double mFreq;
	double mQ;
  public:
	CHPF2(double theFS) : CEA(theFS) {}
	void SetProperty(double theFreq, double theQ) {
		mFreq = theFreq; mQ = theQ;
		double w0 = tan(M_PI * theFreq / FS);

		double den = 1 + w0 / theQ + w0 * w0;
		a0 = 1.0 / den;
		a1 = -2.0 / den;
		a2 = 1.0 / den;
		b1 = -2 * (w0 * w0 - 1) / den;
		b2 = -(1 - w0 / theQ + w0 * w0) / den;
	}
	void ShowProperty() {
		printf("HPF2\n");
		printf(" freq %g (Hz)\n", mFreq);
		printf(" Q    %g\n", mQ);
	}
};

class CLPF2 : public CEA {
	double mFreq;
	double mQ;
  public:
	CLPF2(double theFS) : CEA(theFS) {}
	void SetProperty(double theFreq, double theQ) {
		mFreq = theFreq; mQ = theQ;
		double w0 = tan(M_PI * theFreq / FS);

		double den = 1 + w0 / theQ + w0 * w0;
		a0 = w0 * w0 / den;
		a1 = 2.0 * w0 * w0 / den;
		a2 = w0 * w0 / den;
		b1 = -2 * (w0 * w0 - 1) / den;
		b2 = -(1 - w0 / theQ + w0 * w0) / den;
	}
	void ShowProperty() {
		printf("LPF2\n");
		printf(" freq %g (Hz)\n", mFreq);
		printf(" Q    %g\n", mQ);
	}
};

class CHSV2 : public CEA {
	double mFreq;
	double mGain;
	double mQ;
  public:
	CHSV2(double theFS) : CEA(theFS) {}
	void SetProperty(double theFreq, double theGain, double theQ) {
		mFreq = theFreq; mGain = theGain; mQ = theQ;
		double w0 = tan(M_PI * theFreq / FS);
		double h = dB2Gain(theGain);

		if (theGain >= 0) {
			double den = 1 + w0 / theQ + w0 * w0;
			a0 = (h + sqrt(h) * w0 / theQ + w0 * w0) / den;
			a1 = 2 * (w0 * w0 - h) / den;
			a2 = (h - sqrt(h) * w0 / theQ + w0 * w0) / den;
			b1 = -2 * (w0 * w0 - 1) / den;
			b2 = -(1 - w0 / theQ + w0 * w0) / den;
		} else {
			double den = 1 + sqrt(h) * w0 / theQ + h * w0 * w0;
			a0 = h * (1 + w0 / theQ + w0 * w0) / den;
			a1 = h * 2 * (w0 * w0 - 1) / den;
			a2 = h * (1 - w0 / theQ + w0 * w0) / den;
			b1 = -2 * (h * w0 * w0 - 1) / den;
			b2 = -(1 - sqrt(h) * w0 / theQ + h * w0 * w0) / den;
		}
	}
	void ShowProperty() {
		printf("HSV2\n");
		printf(" freq %g (Hz)\n", mFreq);
		printf(" gain %g (dB)\n", mGain);
		printf(" Q    %g\n", mQ);
	}
};

class CLSV2 : public CEA {
	double mFreq;
	double mGain;
	double mQ;
  public:
	CLSV2(double theFS) : CEA(theFS) {}
	void SetProperty(double theFreq, double theGain, double theQ) {
		mFreq = theFreq; mGain = theGain; mQ = theQ;
		double w0 = tan(M_PI * theFreq / FS);
		double h = dB2Gain(theGain);

		if (theGain >= 0) {
			double den = 1 + w0 / theQ + w0 * w0;
			a0 = (1 + sqrt(h) * w0 / theQ + h * w0 * w0) / den;
			a1 = 2 * (h * w0 * w0 - 1) / den;
			a2 = (1 - sqrt(h) * w0 / theQ + h * w0 * w0) / den;
			b1 = -2 * (w0 * w0 - 1) / den;
			b2 = -(1 - w0 / theQ + w0 * w0) / den;
		} else {
			double den = h + sqrt(h) * w0 / theQ + w0 * w0;
			a0 = h * (1 + w0 / theQ + w0 * w0) / den;
			a1 = h * 2 * (w0 * w0 - 1) / den;
			a2 = h * (1 - w0 / theQ + w0 * w0) / den;
			b1 = -2 * (w0 * w0 - h) / den;
			b2 = -(h - sqrt(h) * w0 / theQ + w0 * w0) / den;
		}
	}
	void ShowProperty() {
		printf("LSV2\n");
		printf(" freq %g (Hz)\n", mFreq);
		printf(" gain %g (dB)\n", mGain);
		printf(" Q    %g\n", mQ);
	}
};

class CPKG2 : public CEA {
	double mFreq;
	double mGain;
	double mQ;
  public:
	CPKG2(double theFS) : CEA(theFS) {}
	void SetProperty(double theFreq, double theGain, double theQ) {
		mFreq = theFreq; mGain = theGain; mQ = theQ;
		double w0 = tan(M_PI * theFreq / FS);
		theQ = (theGain < 0) ? (theQ * dB2Gain(theGain)) : theQ;
		theQ = theQ * theFreq * M_PI / (FS * w0);	// prewarping ?

		double den = 1 + w0 / theQ + w0 * w0;
		a0 = (1 + dB2Gain(theGain) * w0 / theQ + w0 * w0) / den;
		a1 = 2 * (w0 * w0 - 1) / den;
		a2 = (1 - dB2Gain(theGain) * w0 / theQ + w0 * w0) / den;
		b1 = -a1;
		b2 = -(1 - w0 / theQ + w0 * w0) / den;
	}
	void ShowProperty() {
		printf("PKG2\n");
		printf(" freq %g (Hz)\n", mFreq);
		printf(" gain %g (dB)\n", mGain);
		printf(" Q    %g\n", mQ);
	}
};

class CBPF2 : public CEA {
	double mFreq;
	double mGain;
	double mQ;
  public:
	CBPF2(double theFS) : CEA(theFS) {}
	void SetProperty(double theFreq, double theGain, double theQ) {
		mFreq = theFreq; mGain = theGain; mQ = theQ;
		double w0 = tan(M_PI * theFreq / FS);
		theQ = (theGain < 0) ? (theQ * dB2Gain(theGain)) : theQ;
		theQ = theQ * theFreq * M_PI / (FS * w0);	// prewarping ?

		double den = 1 + w0 / theQ + w0 * w0;
		a0 = dB2Gain(theGain) * w0 / theQ / den;
		a1 = 0;
		a2 = -a0;
		b1 = -2 * (w0 * w0 - 1) / den;
		b2 = -(1 - w0 / theQ + w0 * w0) / den;
	}
	void ShowProperty() {
		printf("BPF2\n");
		printf(" freq %g (Hz)\n", mFreq);
		printf(" gain %g (dB)\n", mGain);
		printf(" Q    %g\n", mQ);
	}
};

class CBEF2 : public CEA {
	double mFreq;
	double mGain;
	double mQ;
  public:
	CBEF2(double theFS) : CEA(theFS) {}
	void SetProperty(double theFreq, double theGain, double theQ) {
		mFreq = theFreq; mGain = theGain; mQ = theQ;
		double w0 = tan(M_PI * theFreq / FS);
		double den = 1 + w0 / theQ + w0 * w0;
		a0 = (1 + dB2Gain(theGain) * w0 / theQ + w0 * w0) / den;
		a1 = 2 * (w0 * w0 - 1) / den;
		a2 = (1 - dB2Gain(theGain) * w0 / theQ + w0 * w0) / den;
		b1 = -a1;
		b2 = -(1 - w0 / theQ + w0 * w0) / den;
	}
	void ShowProperty() {
		printf("BEF2\n");
		printf(" freq %g (Hz)\n", mFreq);
		printf(" gain %g (dB)\n", mGain);
		printf(" Q    %g\n", mQ);
	}
};

class CAPF2 : public CEA {
	double mFreq;
	double mQ;
  public:
	CAPF2(double theFS) : CEA(theFS) {}
	void SetProperty(double theFreq, double theQ) {
		mFreq = theFreq; mQ = theQ;
		double w0 = tan(M_PI * theFreq / FS);
		double den = 1 + w0 / theQ + w0 * w0;
		a0 = (1 - w0 / theQ + w0 * w0) / den;
		a1 = 2 * (w0 * w0 - 1) / den;
		a2 = 1;
		b1 = -a1;
		b2 = -(1 - w0 / theQ + w0 * w0) / den;
	}
	void ShowProperty() {
		printf("APF2\n");
		printf(" freq %g (Hz)\n", mFreq);
		printf(" Q    %g\n", mQ);
	}
};


class CSINE : public CFilterBase {
  protected:
	double	FS;
	double  mFreq;
	double  mInitialPhase;
	double 	phase_inc;
	double	mCur_phase;

  public:
	CSINE(double theFS) {
		FS = theFS;
		phase_inc = 0;
		mCur_phase = 0;
	}
	void ShowCoef() {}
	void SetProperty(double theFreq, double theInitialPhaseAsDegree) {
		mCur_phase = theInitialPhaseAsDegree * 2 * M_PI / 360.0;
		phase_inc = 2 * M_PI * theFreq / FS;
		mFreq = theFreq;
		mInitialPhase = theInitialPhaseAsDegree;
	}
	void ShowProperty() {
		printf("SINE\n");
		printf(" freq %g (Hz)\n", mFreq);
		printf(" initial_phase %g (degree)\n", mInitialPhase);
	}
	double Push(double theVal) {
		double aVal = theVal * sin(mCur_phase);
		mCur_phase += phase_inc;
		if (mCur_phase > 2 * M_PI) {
			mCur_phase -= 2 * M_PI;
		}
		return aVal;
	}
};


#endif	/* _filters_h_ */
