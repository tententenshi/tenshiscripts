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
