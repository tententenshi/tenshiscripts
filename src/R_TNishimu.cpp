#include <errno.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>


#include "R_TNishimu.h"

#define         PI      3.14159265358979
#define         M6DB    0.5011872336
#define         FSAMP   32000.0

// ====================================================================================
//	R_TNExclusiveBaseData
// ====================================================================================


// #include "stdafx.h"	// Remove "//" at left end of this line when in Windows VC++

//#include <stdlib.h>
//#include "R_TNExclusiveBaseData.h"

// MIDI packet field constants
static const int filed_MEXBaseAddress = 5;
int gProgram = 0;

//////////////////////////////////////////////////////////////////////////////
// MIDI exclusive basic class
//
R_TNExclusiveBaseData::R_TNExclusiveBaseData()
{
	mPackBodySize = 0;
	mModel_ID = 0x6C;  //RSS-10 
	mDeviceID = 0x10;
	
}

R_TNExclusiveBaseData::~R_TNExclusiveBaseData()
{
}

int R_TNExclusiveBaseData::Pack(unsigned char outBuf[])
{
    unsigned char* outBufPointer;
    unsigned char* outBufAddress;
    outBufPointer = outBufAddress = &outBuf[0];	    
    
            PackHeader( &outBufAddress );
	PackBody( &outBufAddress );
	PackFooter( outBufPointer, &outBufAddress );
	return ( outBufAddress - outBufPointer );
}

void R_TNExclusiveBaseData::PackHeader( unsigned char **outBufHandle )
{
	// MEX header
	**outBufHandle = 0xF0;	 (*outBufHandle)++;
	**outBufHandle = 0x41;	 (*outBufHandle)++;
	**outBufHandle = mDeviceID;	 (*outBufHandle)++;
	**outBufHandle = mModel_ID;	 (*outBufHandle)++;
	**outBufHandle = 0x12;	 (*outBufHandle)++;
	
	// MEX address
	**outBufHandle = mAddress0 + gProgram;	 (*outBufHandle )++;
	**outBufHandle = mAddress1;	 (*outBufHandle )++;
}

void R_TNExclusiveBaseData::PackFooter( unsigned char* outBufPointer, unsigned char** outBufHandle )
{
	// check-sum
	unsigned int theSum = 0;
	unsigned char* theSumPointer;
	theSumPointer = outBufPointer + filed_MEXBaseAddress;
	for ( unsigned int i = 0; i < mPackBodySize + 2; i++)
	{
		theSum += (unsigned int)( *theSumPointer++ );
	}
	**outBufHandle = (unsigned char)(-theSum & 0x7F);   (*outBufHandle)++;

	// 0xF7
	**outBufHandle = (unsigned char)0xF7;	(*outBufHandle)++;

}


//////////////////////////////////////////////////////////////////////////////
// TN_Mixer class
//////////////////////////////////////////////////////////////////////////////
TN_Mixer::TN_Mixer(  int address0, int address1, int direct, int effect  )
{
	mPackBodySize = 2;
	
	mAddress0 = address0; 
	mAddress1 = address1; 
	
	mDirectLevel = direct;
	mEffectLevel = effect;
}

void TN_Mixer::PackBody( unsigned char **outBufHandle )
{	
	**outBufHandle = (unsigned int)( mDirectLevel & 0x7F); (*outBufHandle)++;
	**outBufHandle = (unsigned int)( mEffectLevel & 0x7F); (*outBufHandle)++;
}

void TN_Mixer::sendValue(  )
{
    	unsigned char buf[256];
	int size = Pack(buf);
	emit MIDI_Message(&buf[0], size);
}

void TN_Mixer::saveAllData( QTextStream* file )
{
        file->writeRawBytes ( (char*)( &mDirectLevel ), sizeof( mDirectLevel ) );	
        file->writeRawBytes ( (char*)( &mEffectLevel ), sizeof( mEffectLevel ) );	
	emit Message( 1, QString(" Mixer data ")); ////////////////////////////
}

void TN_Mixer::saveAllDataText( QTextStream* file )
{
	(*file) << "\n                  Direct level  : " << mDirectLevel;
	(*file) << "\n                  Effect level  : " << mEffectLevel;
}

void TN_Mixer::loadAllData( QTextStream* file )
{ 
        file->readRawBytes ( (char*)( &mDirectLevel ), sizeof( mDirectLevel ) );	
	emit Message( 1, QString(" - mDirectLevel : "+ QString::number( mDirectLevel, 10 ))); ////////////////////////////
       file->readRawBytes ( (char*)( &mEffectLevel ), sizeof( mEffectLevel ) );	
	emit Message( 1, QString(" - mEffectLevel : "+ QString::number( mEffectLevel, 10 ))); ////////////////////////////
	
	emit Message( 1, QString(" -------------------- ")); ////////////////////////////
      // file->readRawBytes ( (char*)( & ), sizeof(  ) );
}



//////////////////////////////////////////////////////////////////////////////
// TN_EQ class
//////////////////////////////////////////////////////////////////////////////
TN_EQ::TN_EQ(  int address0, int address1, int type  )
{
	mPackBodySize = 6;
	
	mAddress0 = address0; 
	mAddress1 = address1; 
	mType = type;
	
	mFreqRelative = 100;
	mLevelRelative = 100;
	mQ = 20;
	mFreqLog = 800;
	mLevel = 64;
	mLastFreqLog = -1;
	mLastLevel = -1;
	mLastQ = -1;
	mLastType = -1;
}

void TN_EQ::PackBody( unsigned char **outBufHandle )
{
    	int theFreq =  (int)(mFreq * ((double)mFreqRelative / 100.0));
	if (theFreq > 15000) theFreq = 15000;
	mFreqSent = (double)theFreq;
	
	int theLevel = (int)(mLevel * ((double)mLevelRelative / 100.0));
	if (theLevel > 88) theLevel = 88;
	else if (theLevel < 4) theLevel = 4;
	mLevelSent = ( theLevel - 0x40 ) * 0.5;
	
	mQSent = 0.25 * pow(10, mQ/24.0 * log10( 16 / 0.25 )) ;
	
	**outBufHandle = (unsigned int)(mType & 0x7F); (*outBufHandle)++;
	**outBufHandle = (unsigned int)(theFreq>>14 & 0x7F); (*outBufHandle)++;
	**outBufHandle = (unsigned int)(theFreq>>7 & 0x7F); (*outBufHandle)++;
	**outBufHandle = (unsigned int)(theFreq & 0x7F); (*outBufHandle)++;
	**outBufHandle = (unsigned int)(theLevel & 0x7F); (*outBufHandle)++;
	**outBufHandle = (unsigned int)(mQ & 0x7F); (*outBufHandle)++;
}

void TN_EQ::sendValue(  )
{
    	unsigned char buf[256];
	int size = Pack(buf);
	emit MIDI_Message(&buf[0], size);
}

void TN_EQ::saveAllData( QTextStream* file )
{
        file->writeRawBytes ( (char*)( &mType ), sizeof( mType ) );	
        file->writeRawBytes ( (char*)( &mFreqLog ), sizeof( mFreqLog ) );	
        file->writeRawBytes ( (char*)( &mLevel ), sizeof( mLevel ) );	
        file->writeRawBytes ( (char*)( &mQ ), sizeof( mQ ) );	
        file->writeRawBytes ( (char*)( &mFreqRelative ), sizeof( mFreqRelative ) );	
        file->writeRawBytes ( (char*)( &mLevelRelative ), sizeof( mLevelRelative ) );	
       // file->writeRawBytes ( (char*)( & ), sizeof(  ) );	
	emit Message( 1, QString(" Save EQ data ")); ////////////////////////////
}

void TN_EQ::saveAllDataText( QTextStream* file )
{
	(*file) << "\n                  Type  : " << mType;
	(*file) << "\n                   Freq  : " << mFreq;
 	(*file) << "\n                   Level sent  : " << mLevelSent;
	(*file) << "\n                   Q  : " << mQ << "  =  " << mQSent; 
}

char* TN_EQ::EqEncodeCoefData(double dCoefData)
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

void TN_EQ::EqFaLowPassCal(QTextStream* file, const char** StepName)
{
    float th, t0, w0;
    float xa, xb, xc;

    th = PI * mFreq / FSAMP;
    t0 = 1.0 / FSAMP;
    w0 = 1.0 * FSAMP * tan(th);

    xa = w0 * t0 / (2.0 + w0 * t0);
    xb = xa;
    xc = (2.0 - w0 * t0) / (2.0 + w0 * t0);

    (*file) << "\n	{" << StepName[0] << ",	" << StepName[1] << ",	" << EqEncodeCoefData(xa) << "},"
	<< "/* " << xa << " */ "  << " /* ceq FA 00 13 " << mFreq << " " << mLevelSent << " 1 " << FSAMP << "*/";
    (*file) << "\n	{" << StepName[2] << ",	" << StepName[3] << ",	" << EqEncodeCoefData(xb) << "},"
	    << "/* " << xb << " */ "  ;
    (*file) << "\n	{" << StepName[4] << ",	" << StepName[5] << ",	" << EqEncodeCoefData(xc) << "},"
	    << "/* " << xc << " */ "  ;
    (*file) << "\n";
}

void TN_EQ::EqFaHighPassCal(QTextStream* file, const char** StepName)
{
    float th, t0, w0;
    float xa, xb, xc;

    th = PI * mFreq / FSAMP;
    t0 = 1.0 / FSAMP;
    w0 = 1.0 * FSAMP * tan(th);

    xa = 2.0 / (2.0 + w0 * t0);
    xb = -xa;
    xc = (2.0 - w0 * t0) / (2.0 + w0 * t0);

    (*file) << "\n	{" << StepName[0] << ",	" << StepName[1] << ",	" << EqEncodeCoefData(xa) << "},"
	 << "/* " << xa << " */ "  << " /* ceq FA 00 14 " << mFreq << " " << mLevelSent << " 1 " << FSAMP << "*/";
    (*file) << "\n	{" << StepName[2] << ",	" << StepName[3] << ",	" << EqEncodeCoefData(xb) << "},"
	    << "/* " << xb << " */ "  ;
    (*file) << "\n	{" << StepName[4] << ",	" << StepName[5] << ",	" << EqEncodeCoefData(xc) << "},"
	    << "/* " << xc << " */ "  ;
    (*file) << "\n";
}

void TN_EQ::EqFaLowShelvingCal(QTextStream* file, const char** StepName)
{
    float th, dCos, dSin, dG;
    float k0, d0, d1, d2, xa, xb, xc;

    th = PI * mFreq / FSAMP;
    dCos = cos(2.0 * th);
    dSin = sin(2.0 * th);
    dG = pow(10.0, fabs(mLevelSent) / 200);

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

    if (mLevelSent >= 0.0) {
	
	(*file) << "\n	{" << StepName[0] << ",	" << StepName[1] << ",	" << EqEncodeCoefData(1.0 / xa) << "},"
	 << " /* ceq FA 00 11 " << mFreq << " " << mLevelSent << " 1 " << FSAMP << "*/";
	(*file) << "\n	{" <<StepName[2] << ",	" << StepName[3] << ",	" << EqEncodeCoefData(-xc / xa) << "},";
	(*file) << "\n	{" << StepName[4] << ",	" << StepName[5] << ",	" << EqEncodeCoefData(-xb / xa) << "},";
	    (*file) << "\n";
    } else {
	(*file) << "\n	{" << StepName[0] << ",	" << StepName[1] << ",	" << EqEncodeCoefData(xa) << "},"
	 << " /* ceq FA 00 11 " << mFreq << " " << mLevelSent << " 1 " << FSAMP << "*/";
	(*file) << "\n	{" << StepName[2] << ",	" << StepName[3] << ",	" << EqEncodeCoefData(xb) << "},";
	(*file) << "\n	{" << StepName[4] << ",	" << StepName[5] << ",	" << EqEncodeCoefData(xc) << "},";
	    (*file) << "\n";
    }
}

void TN_EQ::EqFaHiShelvingCal(QTextStream* file, const char** StepName)
{
    float th, dCos, dSin, dG;
    float k0, d0, d1, d2, xa, xb, xc;

    th = PI * mFreq / FSAMP;

    dCos = cos(2.0 * th);
    dSin = sin(2.0 * th);
    dG = pow(10.0, fabs(mLevelSent) / 200);

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

    if (mLevelSent >= 0.0) {
	
	(*file) << "\n	{" << StepName[0] << ",	" << StepName[1] << ",	" << EqEncodeCoefData(1.0 / xa) << "},"
	 << " /* ceq FA 00 12 " << mFreq << " " << mLevelSent << " 1 " << FSAMP << "*/";
	(*file) << "\n	{" <<StepName[2] << ",	" << StepName[3] << ",	" << EqEncodeCoefData(-xc / xa) << "},";
	(*file) << "\n	{" << StepName[4] << ",	" << StepName[5] << ",	" << EqEncodeCoefData(-xb / xa) << "},";
	    (*file) << "\n";
    } else {
	(*file) << "\n	{" << StepName[0] << ",	" << StepName[1] << ",	" << EqEncodeCoefData(xa) << "},"
	 << " /* ceq FA 00 12 " << mFreq << " " << mLevelSent << " 1 " << FSAMP << "*/";
	(*file) << "\n	{" << StepName[2] << ",	" << StepName[3] << ",	" << EqEncodeCoefData(xb) << "},";
	(*file) << "\n	{" << StepName[4] << ",	" << StepName[5] << ",	" << EqEncodeCoefData(xc) << "},";
	    (*file) << "\n";
    }
}

void TN_EQ::EqVePeakingCal(QTextStream* file, const char** StepName)
{
    double dTh, dTanTh, dG;
    double dDD, dPP, dQQ, dRoot, dC, dSq, dQualt;
    
    dQualt = mQSent;
    dTh = mFreq / FSAMP * PI;
    dTanTh = tan(dTh);
    dG = pow(10.0, fabs(mLevelSent) / 200);
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

    (*file) << "\n	{" << StepName[0] << ",	" << StepName[1] << ",	" << EqEncodeCoefData((dPP / dRoot) * (dTanTh * dDD) * dC) << "},"
	 << " /* ceq VE 00 26 " << mFreq << " " << mLevelSent << " " << mQSent << " " << FSAMP << "*/";
    (*file) << "\n	{" << StepName[2] << ",	" << StepName[3] << ",	" << EqEncodeCoefData((dPP / dRoot) * (dTanTh * dDD) * dC) << "},";
    (*file) << "\n	{" << StepName[4] << ",	" << StepName[5] << ",	" << EqEncodeCoefData(dRoot) << "},";
    (*file) << "\n	{" << StepName[6] << ",	" << StepName[7] << ",	" << EqEncodeCoefData((-2.0 * dTanTh * dDD) / dRoot) << "},";
    (*file) << "\n	{" << StepName[8] << ",	" << StepName[9] << ",	" << EqEncodeCoefData(dRoot) << "},";
    (*file) << "\n	{" << StepName[12] << ",	" << StepName[13] << ",	" << EqEncodeCoefData((pow(10.0, (mLevelSent / 200.0)) - 1.0) / (dPP * dC)) << "},";
    (*file) << "\n";
    
}

void TN_EQ::saveAllDataAsCoef( QTextStream* file, const char** StepName)
{
    
    	switch(mType){
                case 0x00:
                        EqFaLowShelvingCal(file, StepName);
                        break;
                case 0x01:
                        EqFaHiShelvingCal(file, StepName);
                        break;
                case 0x02:
                        EqFaLowPassCal(file, StepName);
                        break;
                case 0x03:
                        EqFaHighPassCal(file, StepName);
                        break;
                case 0x04:
                        EqVePeakingCal(file, StepName);
                        break;
             default:
                        break;
                }
}

void TN_EQ::loadAllData( QTextStream* file )
{
	//QString str = "Address:" + QString::number( (long)(this), 16 ).upper() + " ";
	//emit Message( 1, QString( str )); 
    
        file->readRawBytes ( (char*)( &mType ), sizeof( mType ) );	
	emit Message( 1, QString(" - mType mode : "+ QString::number( mType, 10 ))); ////////////////////////////
        file->readRawBytes ( (char*)( &mFreqLog ), sizeof( mFreqLog ) );	
	emit Message( 1, QString(" - mFreqLog : "+ QString::number( mFreqLog, 10 ))); ////////////////////////////
        file->readRawBytes ( (char*)( &mLevel ), sizeof( mLevel ) );	
	emit Message( 1, QString(" - mLevel : "+ QString::number( mLevel, 10 ))); ////////////////////////////
        file->readRawBytes ( (char*)( &mQ ), sizeof( mQ ) );	
	emit Message( 1, QString(" - mQ : "+ QString::number( mQ, 10 ))); ////////////////////////////
        file->readRawBytes ( (char*)( &mFreqRelative ), sizeof( mFreqRelative ) );	
 	emit Message( 1, QString(" - mFreqRelative : "+ QString::number( mFreqRelative, 10 ))); ////////////////////////////
        file->readRawBytes ( (char*)( &mLevelRelative ), sizeof( mLevelRelative ) );	
	emit Message( 1, QString(" - mLevelRelative : "+ QString::number( mLevelRelative, 10 ))); ////////////////////////////
	
	mFreq = (int)( 15* pow( 10, (double) mFreqLog / 640 ) + 0.5 ); 
	mLastFreqLog = mFreqLog;
	mLastLevel = mLevel;
	mLastQ = mQ;
	mLastType = mType;
	
	emit Message( 1, QString(" -------------------- ")); ////////////////////////////
      // file->readRawBytes ( (char*)( & ), sizeof(  ) );
}

//////////////////////////////////////////////////////////////////////////////
// TN_Delay class
//////////////////////////////////////////////////////////////////////////////
TN_Delay::TN_Delay( int address0, int address1,  int max  )
{
	mPackBodySize = 4;
	
	mAddress0 = address0;
	mAddress1 = address1;
	
	mTap = max / 2;
	mLevel = 100;
	mTapRelative = 100;
	mLevelRelative = 100;
	mOutputCh = 0;
	mMaxTap = max;
	
	mLastTap = -1;
	mLastLevel = -1;
	mLastOutputCh = -1;

}

void TN_Delay::PackBody( unsigned char **outBufHandle )
{
    	int theTap =  (int)(mTap * ((double)mTapRelative / 100.0));
	if ( theTap > mMaxTap ) theTap = mMaxTap;
	mTapSent = (double)theTap;
	
	int theLevel = (int)(mLevel * ((double)mLevelRelative / 100.0));
	if (theLevel > 127) theLevel = 127;
	mLevelSent = theLevel / 127.0;
	
	**outBufHandle = (unsigned int)( theTap >> 7 & 0x7F); (*outBufHandle)++;
	**outBufHandle = (unsigned int)( theTap & 0x7F); (*outBufHandle)++;
	**outBufHandle = (unsigned int)( theLevel & 0x7F); (*outBufHandle)++;
	**outBufHandle = (unsigned int)( mOutputCh & 0x7F); (*outBufHandle)++;
}

void TN_Delay::sendValue(  )
{
    	unsigned char buf[256];
	int size = Pack(buf);
	emit MIDI_Message(&buf[0], size);
}

void TN_Delay::saveAllData( QTextStream* file )
{
	file->writeRawBytes ( (char*)( &mTap ), sizeof( mTap ) );    
	file->writeRawBytes ( (char*)( &mLevel ), sizeof( mLevel ) );    
	file->writeRawBytes ( (char*)( &mTapRelative ), sizeof( mTapRelative ) );    
	file->writeRawBytes ( (char*)( &mLevelRelative ), sizeof( mLevelRelative ) );    
	file->writeRawBytes ( (char*)( &mOutputCh ), sizeof( mOutputCh ) );    
}

void TN_Delay::saveAllDataText( QTextStream* file )
{
	(*file) << "\n                   Tap sent : " << mTapSent;
	(*file) << "\n                   Level sent : " << mLevelSent ;	
	(*file) << "\n                   Output ch  : " << mOutputCh;
}

char* TN_Delay::EqEncodeCoefData(double dCoefData)
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

void TN_Delay::saveAllDataAsCoef( QTextStream* file )
{
    
}

void	TN_Delay::saveLevelDataAsCoef( QTextStream* file, const char ** StepName )
{
    (*file) << "\n	{" << StepName[0] << ",	" << StepName[1] << ",	" << EqEncodeCoefData(mLevelSent) << "},";
}
void	TN_Delay::saveOutChDataAsCoef( QTextStream* file, const char ** StepName )
{
    static const char* outCh[] = {
	"IOF_R_TAFRONT_L",
	"IOF_R_TAFRONT_R",
	"IOF_TAREAR_L",
	"IOF_TAREAR_R",
    };
    (*file) << "\n	{" << StepName[0] << ",	" << StepName[1] << ",	" << outCh[mOutputCh] << "},";
}
void TN_Delay::loadAllData( QTextStream* file )
{
	file->readRawBytes ( (char*)( &mTap ), sizeof( mTap ) );    
		emit Message( 1, QString(" - mTap : "+ QString::number( mTap, 10 ))); ////////////////////////////
	file->readRawBytes ( (char*)( &mLevel ), sizeof( mLevel ) );    
		emit Message( 1, QString(" - mLevel : "+ QString::number( mLevel, 10 ))); ////////////////////////////
	file->readRawBytes ( (char*)( &mTapRelative ), sizeof( mTapRelative ) );    
		emit Message( 1, QString(" - mTapRelative : "+ QString::number( mTapRelative, 10 ))); ////////////////////////////
	file->readRawBytes ( (char*)( &mLevelRelative ), sizeof( mLevelRelative ) );    
		emit Message( 1, QString(" - mLevelRelative : "+ QString::number( mLevelRelative, 10 ))); ////////////////////////////
	file->readRawBytes ( (char*)( &mOutputCh ), sizeof( mOutputCh ) );    
		// emit Message( 1, QString(" - mOutputCh : "+ QString::number( mOutputCh, 10 ))); ////////////////////////////
	emit Message( 1, QString(" -------------------- ")); ////////////////////////////
	
	mLastTap = mTap;
	mLastLevel = mLevel;
	mLastOutputCh = mOutputCh;
}

TN_Delay2::TN_Delay2( int address0, int address1,  int max  )
	: TN_Delay( address0, address1, max )
{
    	mLevel = 10000;	
}

void TN_Delay2::PackBody( unsigned char **outBufHandle )
{
    	int theTap =  (int)(mTap * ((double)mTapRelative / 100.0));
	if ( theTap > mMaxTap ) theTap = mMaxTap;
	mTapSent = (double)theTap;
	
	int theLevel = (int)(mLevel * ((double)mLevelRelative / 100.0));
	if (theLevel > 16383 ) theLevel = 16383;
	mLevelSent = theLevel / 16383.0;
	
	**outBufHandle = (unsigned int)( ( theTap >> 7 ) & 0x7F); (*outBufHandle)++;
	**outBufHandle = (unsigned int)( theTap & 0x7F); (*outBufHandle)++;
	**outBufHandle = (unsigned int)( ( theLevel >> 7 ) & 0x7F); (*outBufHandle)++;
	**outBufHandle = (unsigned int)( theLevel & 0x7F); (*outBufHandle)++;
}


void TN_Delay2::saveAllDataText( QTextStream* file )
{
	(*file) << "\n                   Tap sent : " << mTapSent;
	(*file) << "\n                   Level sent : " << mLevelSent ;
}

//////////////////////////////////////////////////////////////////////////////
// TN_Loop class
//////////////////////////////////////////////////////////////////////////////
TN_Loop::TN_Loop( int number )
{
	mPreDelay[0] = new TN_Delay2( number, 0x00, 2999 );
	connect ( mPreDelay[0], SIGNAL(MIDI_Message(unsigned char*, int)), this, SIGNAL(MIDI_Message(unsigned char*, int)));		
	connect ( mPreDelay[0], SIGNAL(Message(const QString&)), this, SIGNAL(Message(const QString&) ));	
	
	mPreDelay[1] = new TN_Delay2( number, 0x04, 2999 );
	connect ( mPreDelay[1], SIGNAL(MIDI_Message(unsigned char*, int)), this, SIGNAL(MIDI_Message(unsigned char*, int)));		
	connect ( mPreDelay[1], SIGNAL(Message(const QString&)), this, SIGNAL(Message(const QString&) ));	
	
	mEQ[0] = new TN_EQ( number, 0x10, 0 );
	connect ( mEQ[0], SIGNAL(MIDI_Message(unsigned char*, int)), this, SIGNAL(MIDI_Message(unsigned char*, int)));		
	connect ( mEQ[0], SIGNAL(Message(const QString&)), this, SIGNAL(Message(const QString&) ));	
	
	mEQ[1] = new TN_EQ( number, 0x18, 0 ); 
	connect ( mEQ[1], SIGNAL(MIDI_Message(unsigned char*, int)), this, SIGNAL(MIDI_Message(unsigned char*, int)));		
	connect ( mEQ[0], SIGNAL(Message(const QString&)), this, SIGNAL(Message(const QString&) ));	
	
	mAllpass = new TN_Delay2( number, 0x30, 1100 );
	connect ( mAllpass, SIGNAL(MIDI_Message(unsigned char*, int)), this, SIGNAL(MIDI_Message(unsigned char*, int)));		
	connect ( mAllpass, SIGNAL(Message(const QString&)), this, SIGNAL(Message(const QString&) ));	
	
	mFeedback = new TN_Delay2( number, 0x34, 7300);
	connect ( mFeedback, SIGNAL(MIDI_Message(unsigned char*, int)), this, SIGNAL(MIDI_Message(unsigned char*, int)));		
	connect ( mFeedback, SIGNAL(Message(const QString&)), this, SIGNAL(Message(const QString&) ));	
	
	mOutput[0] = new TN_Delay( number, 0x40, 7000 );
	connect ( mOutput[0], SIGNAL(MIDI_Message(unsigned char*, int)), this, SIGNAL(MIDI_Message(unsigned char*, int)));		
	connect ( mOutput[0], SIGNAL(Message(const QString&)), this, SIGNAL(Message(const QString&) ));	
	
	mOutput[1] = new TN_Delay( number, 0x44, 7000 );
	connect ( mOutput[1], SIGNAL(MIDI_Message(unsigned char*, int)), this, SIGNAL(MIDI_Message(unsigned char*, int)));		
	connect ( mOutput[1], SIGNAL(Message(const QString&)), this, SIGNAL(Message(const QString&) ));	
	
	mOutput[2] = new TN_Delay( number, 0x48, 7000 );
	connect ( mOutput[2], SIGNAL(MIDI_Message(unsigned char*, int)), this, SIGNAL(MIDI_Message(unsigned char*, int)));		
	connect ( mOutput[2], SIGNAL(Message(const QString&)), this, SIGNAL(Message(const QString&) ));	
	
	mOutput[3] = new TN_Delay( number, 0x4C, 7000 );
	connect ( mOutput[3], SIGNAL(MIDI_Message(unsigned char*, int)), this, SIGNAL(MIDI_Message(unsigned char*, int)));		
	connect ( mOutput[3], SIGNAL(Message(const QString&)), this, SIGNAL(Message(const QString&) ));	
	
    	mAllpassSlaveRatio = 15;
}

void TN_Loop::setAllpass_Tap( int val )
{
	if ( !mAllpassSlave ) {
		mAllpass->setTap( val ); 
	}
}

void TN_Loop::setAllpass_Level( int val ) 
{
	mAllpass->setLevel( val ); 
}

void TN_Loop::setFeedback_Tap( int val )
{
	if ( mAllpassSlave ) {
		int theValue;
		theValue = (int)( val * mAllpassSlaveRatio / 100.0 );
		
		if ( theValue > 1100 ) theValue = 1100;
		mAllpass->setTap( theValue ); 
		//mAllpass->sendValue( ); 
	} 
	mFeedback->setTap( val ); 
}

void TN_Loop::setAllpass_SlaveRatio( int val )
{
	if ( mAllpassSlave ) {
	mAllpassSlaveRatio = val;

	int theValue;
	theValue = (int)( mFeedback->mTap * mAllpassSlaveRatio / 100.0 );
	
	if ( theValue > 1100 ) theValue = 1100;
	mAllpass->setTap( theValue ); 
		//mAllpass->sendValue( ); 
    	}
}



void TN_Loop::setFeedback_Level( int val )
{
/*
    	if ( mAllpassSlave ) {
	    int theValue;
	    theValue = val * mAllpassSlaveRatio / 100.0;
		mAllpass->setLevel( theValue ); 
		mAllpass->sendValue( ); 
	} 
*/
	mFeedback->setLevel( val ); 
	//mFeedback->sendValue( );
}

void TN_Loop::saveAllData( QTextStream* file )
{
	file->writeRawBytes ( (char*)( &mAllpassSlave ), sizeof( mAllpassSlave ) );
	file->writeRawBytes ( (char*)( &mAllpassSlaveRatio ), sizeof( mAllpassSlaveRatio ) );
	
	mPreDelay[ 0 ]->saveAllData( file );
	mPreDelay[ 1 ]->saveAllData( file );
	
	mEQ[ 0 ]->saveAllData( file );
	mEQ[ 1 ]->saveAllData( file );
	
	mAllpass->saveAllData( file );
	mFeedback->saveAllData( file );
	
	mOutput[ 0 ]->saveAllData( file );
	mOutput[ 1 ]->saveAllData( file );
	mOutput[ 2 ]->saveAllData( file );
	mOutput[ 3 ]->saveAllData( file );
}
////////////////////////////////////////////////// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
void TN_Loop::saveAllDataText( QTextStream* file )
{
	//file->writeRawBytes ( (char*)( &mAllpassSlave ), sizeof( mAllpassSlave ) );
	(*file) << "\n        Allpass slave mode : " << mAllpassSlave;
	// file->writeRawBytes ( (char*)( &mAllpassSlaveRatio ), sizeof( mAllpassSlaveRatio ) );
	(*file) << "\n        Allpass slave ratio : " << mAllpassSlaveRatio;
	
	(*file) << "\n          ----- Loop pre delay Lch---- : ";
	mPreDelay[ 0 ]->saveAllDataText( file );
	(*file) << "\n          ----- Loop pre delay Rch---- : ";
	mPreDelay[ 1 ]->saveAllDataText( file );
	
	(*file) << "\n          ----- Loop EQ 0 ---- : ";
	mEQ[ 0 ]->saveAllDataText( file );
	(*file) << "\n          ----- Loop EQ 1 ---- : ";
	mEQ[ 1 ]->saveAllDataText( file );
	
	(*file) << "\n          ----- Loop allpass ---- : ";
	mAllpass->saveAllDataText( file );
	(*file) << "\n          ----- Loop feedback ---- : ";
	mFeedback->saveAllDataText( file );
	
	(*file) << "\n          ----- Loop output 0 ---- : ";
	mOutput[ 0 ]->saveAllDataText( file );
	(*file) << "\n          ----- Loop output 1 ---- : ";
	mOutput[ 1 ]->saveAllDataText( file );
	(*file) << "\n          ----- Loop output 2 ---- : ";
	mOutput[ 2 ]->saveAllDataText( file );
	(*file) << "\n          ----- Loop output 3 ---- : ";
	mOutput[ 3 ]->saveAllDataText( file );
}
void TN_Loop::saveEQ0DataAsCoef( QTextStream* file, const char ** StepName )
{
	//(*file) << "\n          ----- Loop EQ 0 ---- : ";
	mEQ[ 0 ]->saveAllDataAsCoef( file , StepName);
}
void TN_Loop::saveEQ1DataAsCoef( QTextStream* file, const char ** StepName )
{
	//(*file) << "\n          ----- Loop EQ 1 ---- : ";
	mEQ[ 1 ]->saveAllDataAsCoef( file , StepName);
}
void TN_Loop::saveIramDataAsCoef(QTextStream* file, const char ** StepName )
{
    	mOutput[ 0 ]->saveOutChDataAsCoef( file , &StepName[0]);
    	mOutput[ 1 ]->saveOutChDataAsCoef( file , &StepName[2]);
	if (gProgram) {
    	mOutput[ 2 ]->saveOutChDataAsCoef( file , &StepName[4]);
    	mOutput[ 3 ]->saveOutChDataAsCoef( file , &StepName[6]);
    }
}
void	TN_Loop::saveBADataAsCoef(QTextStream* file, const char ** StepName )
{
    	mOutput[ 0 ]->saveLevelDataAsCoef( file , &StepName[0]);
    	mOutput[ 1 ]->saveLevelDataAsCoef( file , &StepName[2]);
	if (gProgram) {
    	mOutput[ 2 ]->saveLevelDataAsCoef( file , &StepName[4]);
    	mOutput[ 3 ]->saveLevelDataAsCoef( file , &StepName[6]);
    }
}

void	TN_Loop::savePD0DataAsCoef(QTextStream* file, const char ** StepName )
{
    	mPreDelay[ 0 ]->saveLevelDataAsCoef( file , StepName);
}
void	TN_Loop::savePD1DataAsCoef(QTextStream* file, const char ** StepName )
{
    	mPreDelay[ 1 ]->saveLevelDataAsCoef( file , StepName);
}
void	TN_Loop::saveCoef8DataAsCoef(QTextStream* file, const char ** StepName )
{
	mAllpass->saveLevelDataAsCoef( file , StepName);
}
void	TN_Loop::saveFBKDataAsCoef(QTextStream* file, const char ** StepName )
{
    	mFeedback->saveLevelDataAsCoef( file , StepName);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TN_Loop::sendAllData(  )
{	
	mPreDelay[ 0 ]->sendValue(  );
	mPreDelay[ 1 ]->sendValue(  );
	
	mEQ[ 0 ]->sendValue(  );
	mEQ[ 1 ]->sendValue(  );
	
	mAllpass->sendValue(  );
	mFeedback->sendValue(  );
	
	mOutput[ 0 ]->sendValue(  );
	mOutput[ 1 ]->sendValue(  );
	mOutput[ 2 ]->sendValue(  );
	mOutput[ 3 ]->sendValue(  );
}
    
void TN_Loop::loadAllData( QTextStream* file )
{
	emit Message( 1, QString(" Load Loop data ")); ////////////////////////////
	file->readRawBytes ( (char*)( &mAllpassSlave ), sizeof( mAllpassSlave ) );
		emit Message( 1, QString(" - Allpass slave mode : "+ QString::number( mAllpassSlave, 10 ))); ////////////////////////////
	file->readRawBytes ( (char*)( &mAllpassSlaveRatio ), sizeof( mAllpassSlaveRatio ) );
		emit Message( 1, QString(" - Allpass slave ratio : "+ QString::number( mAllpassSlaveRatio, 10 ))); ////////////////////////////
	emit Message( 1, QString(" -------------------- ")); ////////////////////////////
		
	emit Message( 1, QString(" Load Predelay 0  ...... ")); ////////////////////////////
	mPreDelay[ 0 ]->loadAllData( file );
	emit Message( 1, QString(" Load Predelay 1  ...... ")); ////////////////////////////
	mPreDelay[ 1 ]->loadAllData( file );
			
	emit Message( 1, QString(" Load Loop EQ 0  ...... ")); ////////////////////////////
	mEQ[ 0 ]->loadAllData( file );
	emit Message( 1, QString(" Load Loop EQ 1  ...... ")); ////////////////////////////
	mEQ[ 1 ]->loadAllData( file );
	
	emit Message( 1, QString(" Load Allpass  ...... ")); ////////////////////////////
	mAllpass->loadAllData( file );
	emit Message( 1, QString(" Load Feedback  ...... ")); ////////////////////////////
	mFeedback->loadAllData( file );
	
	emit Message( 1, QString(" Load Output 0  ...... ")); ////////////////////////////
	mOutput[ 0 ]->loadAllData( file );
	emit Message( 1, QString(" Load Output 1  ...... ")); ////////////////////////////
	mOutput[ 1 ]->loadAllData( file );
	emit Message( 1, QString(" Load Output 2  ...... ")); ////////////////////////////
	mOutput[ 2 ]->loadAllData( file );
	emit Message( 1, QString(" Load Output 3  ...... ")); ////////////////////////////
	mOutput[ 3 ]->loadAllData( file );

}
   
// ==============================================================================
#include <qslider.h>
#include <qpushbutton.h>
#include <qlayout.h>

R_TNishimu::R_TNishimu( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    {
	mOnDisplay[ 0 ] = true;
	for (int i=1; i<16; i++ ) {
    		mOnDisplay[ i ] = false;
	    }
    }
    	mDataVersion = 30728001;
    
	QVBoxLayout *layout = new QVBoxLayout( this );
	layout->setMargin( 0 );
    
	mTextEdit = new QTextEdit( this,"Access Log" );
    	layout->addWidget( mTextEdit );   
    	connect ( this, SIGNAL( Message(const QString&) ), mTextEdit, SLOT( append(const QString&)  )); 
    	connect ( this, SIGNAL( Message(int, const QString&) ), this, SLOT( ShowMessage( int, const QString&)  )); 
    	
	mPreEQControlMode = 0;	// 0:Both, 1:Lch, 2:Rch
	mPreEQSelectSingleNumber = 0;	// 0,1,2,3,,,,,,15
	mPreEQSelectMulti = false;	// 0: Single moe, 1: Multi mode
	
	mLoopSelectSingleNumber = 0;	// 0,1,2,3,4,5,6,7,,,,,15
	mLoopSelectMulti = 0;	// 0: Single moe, 1: Multi mode
	
	{
		int i;
		for ( int m = 0; m<3; m++ ) {
			for ( i = 0; i<8; i++ ) {
		 		mEQ[ m ][ i ] = new TN_EQ( 0x20+m, 0x08*i, 0); 
				connect ( mEQ[ m ][ i ], SIGNAL(MIDI_Message(unsigned char*, int)), this, SIGNAL(MIDI_Message(unsigned char*, int)));	
				connect ( mEQ[ m ][ i ], SIGNAL(Message(const QString&)), this, SIGNAL(Message(const QString&) ));	
				mPreEQSelect[ i ] = true;	// surfix: EQnumber,  sw 0:Off,  1:On
			} 
			for (i = 8; i<16; i++ ) {
	 			mEQ[ m ][ i ] = new TN_EQ( 0x20+m, 0x08*i, 4); 
				connect ( mEQ[ m ][ i ], SIGNAL(MIDI_Message(unsigned char*, int)), this, SIGNAL(MIDI_Message(unsigned char*, int)));	
				connect ( mEQ[ m ][ i ], SIGNAL(Message(const QString&)), this, SIGNAL(Message(const QString&) ));	
				mPreEQSelect[ i ] = true;	// surfix: EQnumber,  sw 0:Off,  1:On
			}
		}
    	}
	
	{
		int i;
		for ( i = 0; i<16; i++ ) {
	 		mLoop[ i ] = new TN_Loop( 0x24 + i ); 
			connect ( mLoop[ i ], SIGNAL(MIDI_Message(unsigned char*, int)), this, SIGNAL(MIDI_Message(unsigned char*, int)));	
			connect ( mLoop[ i ], SIGNAL(Message(const QString&)), this, SIGNAL(Message(const QString&) ));	
			mLoopSelect[ i ] = true; 	// surfix: EQnumber,  sw 0:Off,  1:On
		} 
	 }
	
	mTrans = new TN_Delay( 0x34, 0x00, 1000 );
	connect ( mTrans, SIGNAL(MIDI_Message(unsigned char*, int)), this, SIGNAL(MIDI_Message(unsigned char*, int)));	
	connect ( mTrans, SIGNAL(Message(const QString&)), this, SIGNAL(Message(const QString&) ));	
	
	
	mMixer_Effect_Off = new TN_Mixer( 0x38, 00, 127, 0 );
	connect ( mMixer_Effect_Off, SIGNAL(MIDI_Message(unsigned char*, int)), this, SIGNAL(MIDI_Message(unsigned char*, int)));	
	connect ( mMixer_Effect_Off, SIGNAL(Message(const QString&)), this, SIGNAL(Message(const QString&) ));	
	mMixer_Effect_On = new TN_Mixer( 0x38, 00, 100, 70 );
	connect ( mMixer_Effect_On, SIGNAL(MIDI_Message(unsigned char*, int)), this, SIGNAL(MIDI_Message(unsigned char*, int)));	
	connect ( mMixer_Effect_On, SIGNAL(Message(const QString&)), this, SIGNAL(Message(const QString&) ));	
	
	
    //setFocusProxy( Slider1 );
    // init();
	 emit Message(QString("TN: Ready "));
	emit ReadyToStartMIDI();
	
	//EmitChangedLoopStatus(  );
	//EmitChangedPreEQStatus( );
	//ShowData();
	
	//QString str = "Address:" + QString::number( (long)(&mEQ[ 0 ][ 0 ]), 16 ).upper() + " ";
	//emit Message( 1, QString( str )); 

}

R_TNishimu::~R_TNishimu()
{
	emit ReadyToStopMIDI();
}
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pre-EQ

void R_TNishimu::setPreEQ_Type( int val )
{
	if ( !mPreEQSelectMulti ) {
		mEQ[ mPreEQControlMode ][ mPreEQSelectSingleNumber ]->setType( val );
	} else {
	    	for ( int i=0; i<10; i++) {
			if ( mPreEQSelect[ i ] ) {
				mEQ[ mPreEQControlMode ][ i ]->setType( val );
	       		} 
	    	}
		mEQ[ mPreEQControlMode ][15 ]->setType( val );
	}
	emit Message( 2, QString("Pre EQ type H / L  ")); /////////////////////////
}

void R_TNishimu::setPreEQ_Freq( int val )
{
	if ( !mPreEQSelectMulti ) {
		mEQ[ mPreEQControlMode ][ mPreEQSelectSingleNumber ]->setFreqLog( val );
		// ###################################
		emit Changed_PreEQ_Freq( mEQ[ mPreEQControlMode ][ mPreEQSelectSingleNumber ]->mFreqSent );
		emit Message( 2, QString("Pre-EQ Freq SINGLE : " + QString::number( mPreEQSelectSingleNumber, 10 ) ) ); /////////////////////////
	} else {
		for ( int i=0; i<10; i++) {
			if ( mPreEQSelect[ i ] ) {
				mEQ[ mPreEQControlMode ][ i ]->setFreqLog( val );
	       		} 
	    	}
		mEQ[ mPreEQControlMode ][ 15 ]->setFreqLog( val );
		// ###################################
		emit Changed_PreEQ_Freq( mEQ[ mPreEQControlMode ][ 15 ]->mFreqSent );
		emit Message( 2, QString("Pre-EQ Freq MULTI : " + QString::number( 15, 10 ) ) ); /////////////////////////
	}
	emit Message( 2, QString("Pre-EQ Freqency  ")); /////////////////////////
}

void R_TNishimu::setPreEQ_Freq_Relative( int val )
{
	for ( int i=0; i<10; i++) {
		mEQ[ mPreEQControlMode ][ i ]->setFreq_Relative( val );	//////////////////////////// /!!!!!!!1
	}
	mEQ[ mPreEQControlMode ][ 15 ]->setFreq_Relative( val );	//////////////////////////// /!!!!!!!1
	emit Message( 2, QString("Loop Pre-EQ Freq relative  ")); /////////////////////////
}

void R_TNishimu::setPreEQ_Level( int val )
{
	if ( !mPreEQSelectMulti ) {
		mEQ[ mPreEQControlMode ][ mPreEQSelectSingleNumber ]->setLevel( val );
		// ###################################
		emit Changed_PreEQ_Level( mEQ[ mPreEQControlMode ][ mPreEQSelectSingleNumber ]->mLevelSent );
		emit Message( 2, QString("Pre-EQ level SINGLE : " + QString::number( mPreEQSelectSingleNumber, 10 ) ) ); /////////////////////////
	} else {
		for ( int i=0; i<10; i++) {
			if ( mPreEQSelect[ i ] ) {
				mEQ[ mPreEQControlMode ][ i ]->setLevel( val );
	       		} 
	    	}
		mEQ[ mPreEQControlMode ][15 ]->setLevel( val );
	    
	   	// ###################################
		emit Changed_PreEQ_Level( mEQ[ mPreEQControlMode ][ 15 ]->mLevelSent );
		emit Message( 2, QString("Pre-EQ level MULTI : " + QString::number( 15, 10 ) ) ); /////////////////////////
	}
}

void R_TNishimu::setPreEQ_Level_Relative( int val )
{
	for ( int i=0; i<10; i++) {
		mEQ[ mPreEQControlMode ][ i ]->setLevel_Relative( val );	//////////////////////////// /!!!!!!!1
	}
	mEQ[ mPreEQControlMode ][ 15 ]->setLevel_Relative( val );	//////////////////////////// /!!!!!!!1
	emit Message( 2, QString("Loop Pre-EQ Level relative  ")); /////////////////////////
}

void R_TNishimu::setPreEQ_Q( int val )
{
	if ( !mPreEQSelectMulti ) {
		mEQ[ mPreEQControlMode ][ mPreEQSelectSingleNumber ]->setQ( val );
		// ###################################
		emit Changed_PreEQ_Q( mEQ[ mPreEQControlMode ][ mPreEQSelectSingleNumber ]->mQSent );
		emit Message( 2, QString("Pre-EQ Q SINGLE : " + QString::number( mPreEQSelectSingleNumber, 10 ) ) ); /////////////////////////
	} else {
		for ( int i=0; i<10; i++) {
			if ( mPreEQSelect[ i ] ) {
				mEQ[ mPreEQControlMode ][ i ]->setQ( val );
	       		} 
	    	}
		mEQ[ mPreEQControlMode ][ 15 ]->setQ( val );
		// ###################################
		emit Changed_PreEQ_Q( mEQ[ mPreEQControlMode ][ 15 ]->mQSent );
		emit Message( 2, QString("Pre-EQ Q MULTI : " + QString::number( 15, 10 ) ) ); /////////////////////////
	}
	emit Message( 2, QString("Pre-EQ Quality  ")); /////////////////////////
}

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Loop Pre-Delay
void R_TNishimu::setLoopPreDelay_Tap( int ch, int val )
{
	emit Message( 2, QString("Loop Pre-delay tap   ")); /////////////////////////
	if ( !mLoopSelectMulti ) {
		mLoop[ mLoopSelectSingleNumber ]->setPreDelay_Tap( ch, val );
		// ###################################
		if (ch==0) emit Changed_LoopPreDelay_0_Tap( mLoop[ mLoopSelectSingleNumber ]->getLoopPreDelay_Tap( 0 ) );
		else emit Changed_LoopPreDelay_1_Tap( mLoop[ mLoopSelectSingleNumber ]->getLoopPreDelay_Tap( 1 ) );
		emit Message( 2, QString("Loop Pre-Delay SINGLE : " + QString::number( mLoopSelectSingleNumber, 10 ) ) ); /////////////////////////
	} else {
		emit Message( 2, QString("Loop Pre-Delay MULTI : " + QString::number( 15, 10 ) ) ); /////////////////////////
		mLoop[ 15 ]->setPreDelay_Tap( ch, val );
		for ( int i=0; i<7; i++) {
			if ( mLoopSelect[ i ] ) {
				mLoop[ i ]->setPreDelay_Tap( ch, val );
	       		} 
			QString str = "Select: " + QString::number((int)(mLoopSelect[ i ]), 10 ).upper() + " ";
			emit Message( 2, QString( str )); 
		}
		
		// ###################################
		if (ch==0) emit Changed_LoopPreDelay_0_Tap( mLoop[ 15 ]->getLoopPreDelay_Tap( 0 ) );
		else emit Changed_LoopPreDelay_1_Tap( mLoop[ 15 ]->getLoopPreDelay_Tap( 1 ) );
	}
}

void R_TNishimu::setLoopPreDelay_Tap_Relative( int ch, int val )
{
	emit Message( 2, QString("Loop Pre-delay tap relative  ")); /////////////////////////
	for ( int i=0; i<7; i++) {
		mLoop[ i ]->setPreDelay_Tap_Relative( ch, val );	//////////////////////////// /!!!!!!!1
	}
	mLoop[ 15 ]->setPreDelay_Tap_Relative( ch, val );	//////////////////////////// /!!!!!!!1
	
	int num;
	if ( !mLoopSelectMulti ) {
	    	num = mLoopSelectSingleNumber;
	} else {
		num = 15;
	}
		// ###################################
		if (ch==0) emit Changed_LoopPreDelay_0_Tap( mLoop[ num ]->getLoopPreDelay_Tap( 0 ) );
		else emit Changed_LoopPreDelay_1_Tap( mLoop[ num ]->getLoopPreDelay_Tap( 1 ) );
		
		//if (ch==0) emit Changed_LoopPreDelay_0_Tap_Slider( mLoop[ num ]->getLoopPreDelay_Tap_Slider( 0 ) );
		//else emit Changed_LoopPreDelay_1_Tap_Slider( mLoop[ num ]->getLoopPreDelay_Tap_Slider( 1 ) );	
		
}


void R_TNishimu::setLoopPreDelay_Level( int ch, int val )
{
	emit Message( 2, QString("Loop Pre-delay level   ")); /////////////////////////
	if ( !mLoopSelectMulti ) {
		mLoop[ mLoopSelectSingleNumber ]->setPreDelay_Level( ch, val );
		// ###################################
		if (ch==0) emit Changed_LoopPreDelay_0_Level( mLoop[ mLoopSelectSingleNumber ]->getLoopPreDelay_Level( 0 ) );
		else emit Changed_LoopPreDelay_1_Level( mLoop[ mLoopSelectSingleNumber ]->getLoopPreDelay_Level( 1 ) );
		emit Message( 2, QString( "Pre-Delay level SINGLE : " + QString::number( mLoopSelectSingleNumber, 10 ) ) ); /////////////////////////
	} else {
		for ( int i=0; i<7; i++) {
			if ( mLoopSelect[ i ] ) {
				mLoop[ i ]->setPreDelay_Level( ch, val );
	       		} 
	    	}
		mLoop[ 15 ]->setPreDelay_Level( ch, val );
		// ###################################
		if (ch==0) emit Changed_LoopPreDelay_0_Level( mLoop[ 15 ]->getLoopPreDelay_Level( 0 ) );
		else emit Changed_LoopPreDelay_1_Level( mLoop[ 15 ]->getLoopPreDelay_Level( 1 ) );
		emit Message( 2, QString( "Pre-Delay level MULTI : " + QString::number( 15, 10 ) ) ); /////////////////////////
	}
}

void R_TNishimu::setLoopPreDelay_Level_Relative( int ch, int val )
{
	emit Message( 2, QString("Loop Pre-delay level relative  ")); /////////////////////////
	for ( int i=0; i<7; i++) {
		mLoop[ i ]->setPreDelay_Level_Relative( ch, val ); //////////////////////////// !!!!!!!!!!
	}
	mLoop[ 15 ]->setPreDelay_Level_Relative( ch, val ); //////////////////////////// !!!!!!!!!!
	
	int num;
	if ( !mLoopSelectMulti ) {
	    	num = mLoopSelectSingleNumber;
	} else {
		num = 15;
	}
		// ###################################
		if (ch==0) emit Changed_LoopPreDelay_0_Level( mLoop[ num ]->getLoopPreDelay_Level( 0 ) );
		else emit Changed_LoopPreDelay_1_Level( mLoop[ num ]->getLoopPreDelay_Level( 1 ) );
		
		//if (ch == 0) emit Changed_LoopPreDelay_0_Level_Slider( mLoop[ num ]->getLoopPreDelay_Level_Slider( 0 ) );
		//else emit Changed_LoopPreDelay_1_Level_Slider( mLoop[ num ]->getLoopPreDelay_Level_Slider( 1 ) );
}

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Loop EQ

void R_TNishimu::setLoopEQ_Type( int num, int val )
{
	if ( !mLoopSelectMulti ) {
		mLoop[ mLoopSelectSingleNumber ]->setEQ_Type( num, val );
		// ###################################
		if ( num ==0 ) emit Changed_LoopEQ_Type( 0, mLoop[ mLoopSelectSingleNumber ]->getEQ_Type( 0 ) );
		else emit Changed_LoopEQ_Type( 1, mLoop[ mLoopSelectSingleNumber ]->getEQ_Type( 1 ) );
		emit Message( 2, QString("Loop-EQ level SINGLE : " + QString::number( mLoopSelectSingleNumber, 10 ) ) ); /////////////////////////
	} else {
		for ( int i=0; i<7; i++) {
			if ( mLoopSelect[ i ] ) {
				mLoop[ i ]->setEQ_Type( num, val );
	       		} 
	    	}
		mLoop[ 15 ]->setEQ_Type( num, val );
			
		if ( num ==0 ) emit Changed_LoopEQ_Type( 0, mLoop[ 15 ]->getEQ_Type( 0 ) );
		else emit Changed_LoopEQ_Type( 1, mLoop[ 15 ]->getEQ_Type( 1 ) );
		emit Message( 2, QString("Loop-EQ level MULTI : " + QString::number( 15, 10 ) ) ); /////////////////////////
	}
}
    
void R_TNishimu::setLoopEQ_FreqLog( int num, int val )
{
	emit Message( 2, QString("Loop EQ freq  ")); /////////////////////////
	if ( !mLoopSelectMulti ) {
		mLoop[ mLoopSelectSingleNumber ]->setEQ_FreqLog( num, val );
		// ##############################
		if (num == 0) emit Changed_LoopEQ_0_Freq( mLoop[ mLoopSelectSingleNumber ]->getEQ_Freq( 0 ) );
		else emit Changed_LoopEQ_1_Freq( mLoop[ mLoopSelectSingleNumber ]->getEQ_Freq( 1 ) );
		emit Message( 2, QString( "Loop-EQ Freq SINGLE : " + QString::number( mLoopSelectSingleNumber, 10 ) ) ); /////////////////////////

	    } else {
	    for ( int i=0; i<7; i++) {
		if ( mLoopSelect[ i ] ) {
			mLoop[ i ]->setEQ_FreqLog( num, val );
	       	} 
	    }
			mLoop[ 15 ]->setEQ_FreqLog( num, val );
		// ##############################		
		if (num == 0) emit Changed_LoopEQ_0_Freq( mLoop[ 15 ]->getEQ_Freq( 0 ) );
		else emit Changed_LoopEQ_1_Freq( mLoop[ 15 ]->getEQ_Freq( 1 ) );
		emit Message( 2, QString( "Loop-EQ Freq MULTI : " + QString::number( 15, 10 ) ) ); /////////////////////////
	}
		//if (num==0) emit Changed_LoopEQ_0_Freq_Slider( mLoop[ mLoopSelectSingleNumber ]->getEQ_Freq_Slider( 0 ) );
		//else emit Changed_LoopEQ_1_Freq_Slider( mLoop[ mLoopSelectSingleNumber ]->getEQ_Freq_Slider( 1 ) );
}

void R_TNishimu::setLoopEQ_Freq_Relative( int num, int val )
{
	emit Message( 2, QString("Loop EQ Freq relative  ")); /////////////////////////
	for ( int i=0; i<7; i++) {
		mLoop[ i ]->setEQ_Freq_Relative( num, val );
	}
	mLoop[ 15 ]->setEQ_Freq_Relative( num, val );
	
	int theSelect;
	if ( !mLoopSelectMulti ) {
	    	theSelect = mLoopSelectSingleNumber;
	} else {
		theSelect = 15;
	}
		// ##############################
		if (num == 0) emit Changed_LoopEQ_0_Freq( mLoop[ theSelect ]->getEQ_Freq( 0 ) );
		else emit Changed_LoopEQ_1_Freq( mLoop[ theSelect ]->getEQ_Freq( 1 ) );
		//if (num==0) emit Changed_LoopEQ_0_Freq_Slider( mLoop[ theSelect ]->getEQ_Freq_Slider( num ) );
		//else emit Changed_LoopEQ_1_Freq_Slider( mLoop[ theSelect ]->getEQ_Freq_Slider( num ) );
		emit Message( 2, QString( "Loop-EQ Freq relative select no. : " + QString::number( theSelect, 10 ) ) ); /////////////////////////
}

void R_TNishimu::setLoopEQ_Level( int num, int val )
{
	emit Message( 2, QString("Loop EQ level  ")); /////////////////////////
	if ( !mLoopSelectMulti ) {
		mLoop[ mLoopSelectSingleNumber ]->setEQ_Level( num, val );
		// ##############################		
		if (num == 0) emit Changed_LoopEQ_0_Level( mLoop[ mLoopSelectSingleNumber ]->getEQ_Level( 0 ) );
		else emit Changed_LoopEQ_1_Level( mLoop[ mLoopSelectSingleNumber ]->getEQ_Level( 1 ) );
		emit Message( 2, QString( "Loop-EQ level SINGLE : " + QString::number( mLoopSelectSingleNumber, 10 ) ) ); /////////////////////////
	} else {
		for ( int i=0; i<7; i++) {
			if ( mLoopSelect[ i ] ) {
				mLoop[ i ]->setEQ_Level( num, val );
	       		} 
	    	}
		mLoop[ 15 ]->setEQ_Level( num, val );
		// ##############################		
		if (num == 0) emit Changed_LoopEQ_0_Level( mLoop[ 15 ]->getEQ_Level( 0 ) );
		else emit Changed_LoopEQ_1_Level( mLoop[ 15 ]->getEQ_Level( 1 ) );
		emit Message( 2, QString( "Loop-EQ level MULTI : " + QString::number( 15, 10 ) ) ); /////////////////////////
	}
}

void R_TNishimu::setLoopEQ_Level_Relative( int num, int val )
{
	emit Message( 2, QString("Loop EQ level relative  ")); /////////////////////////
	for ( int i=0; i<7; i++) {
		mLoop[ i ]->setEQ_Level_Relative( num, val );
	}
	mLoop[ 15 ]->setEQ_Level_Relative( num, val );
	
	int theSelect;
	if ( !mLoopSelectMulti ) {
	    	theSelect = mLoopSelectSingleNumber;
	} else {
		theSelect = 15;
	}
		// ##############################		
		if (num == 0) emit Changed_LoopEQ_0_Level( mLoop[ theSelect ]->getEQ_Level( 0 ) );
		else emit Changed_LoopEQ_1_Level( mLoop[ theSelect ]->getEQ_Level( 1 ) );
		////emit Message( 2, QString( "Loop-EQ Level relative select no. = " + QString::number( theSelect, 10 ) + " Level = " + QString::number( (int)(mLoop[ theSelect ]->getEQ_Level( 0 )), 10 )  ) ); /////////////////////////
			
	//	if (num==0) emit Changed_LoopEQ_0_Level_Slider( mLoop[ theSelect ]->getEQ_Level_Slider( 0 ) );
	//	else emit Changed_LoopEQ_1_Level_Slider( mLoop[ theSelect ]->getEQ_Level_Slider( 1 ) );
}

// /////////////////////////////////////////////////////////////////////////////////  
// Loop Allpass
void R_TNishimu::setLoopAllpass_Tap( int val )
{
	emit Message( 2, QString("Loop allpass tap  ")); /////////////////////////
	if ( !mLoopSelectMulti ) {
		mLoop[ mLoopSelectSingleNumber ]->setAllpass_Tap( val );
		// ##############################		
		emit Changed_Allpass_Tap( mLoop[ mLoopSelectSingleNumber ]->getAllpass_Tap() );
		emit Message( 2, QString( "Allpass Tap SINGLE : " + QString::number( mLoopSelectSingleNumber, 10 ) ) ); /////////////////////////
	} else {
		for ( int i=0; i<7; i++) {
			if ( mLoopSelect[ i ] ) {
				mLoop[ i ]->setAllpass_Tap( val );
	       		} 
	    	}
		mLoop[ 15 ]->setAllpass_Tap( val );
		// ##############################		
		emit Changed_Allpass_Tap( mLoop[ 15 ]->getAllpass_Tap() );
		emit Message( 2, QString( "Allpass Tap MULTI : " + QString::number( 15, 10 ) ) ); /////////////////////////
	}
}

void R_TNishimu::setLoopAllpass_Slave( bool sw )
{
	emit Message( 2, QString("Loop allpass slave ")); /////////////////////////
	if ( !mLoopSelectMulti ) {
		mLoop[ mLoopSelectSingleNumber ]->setAllpass_Slave( sw );
		// ##############################		
		emit Changed_Allpass_Tap( mLoop[ mLoopSelectSingleNumber ]->getAllpass_Tap() );
		emit Changed_Allpass_Tap_Slider( mLoop[ mLoopSelectSingleNumber ]->getAllpass_Tap_Slider() );
		emit Message( 2, QString( "Allpass slave SINGLE : " + QString::number( mLoopSelectSingleNumber, 10 ) ) ); /////////////////////////
	} else {
	    for ( int i=0; i<7; i++) {
		if ( mLoopSelect[ i ] ) {
			mLoop[ i ]->setAllpass_Slave( sw );
	       	} 
	    }
			mLoop[ 15 ]->setAllpass_Slave( sw );
		// ##############################		
		emit Changed_Allpass_Tap( mLoop[ 15 ]->getAllpass_Tap() );
		emit Message( 2, QString( "Allpass slave MULTI : " + QString::number( 15, 10 ) ) ); /////////////////////////
		emit Changed_Allpass_Tap_Slider( mLoop[ 15 ]->getAllpass_Tap_Slider() );
	}
}

void R_TNishimu::setLoopAllpass_SlaveRatio( int val )
{
	emit Message( 2, QString("Loop allpass slave ratio ")); /////////////////////////
	if ( !mLoopSelectMulti ) {
		mLoop[ mLoopSelectSingleNumber ]->setAllpass_SlaveRatio( val );
			// ##############################		
		emit Changed_Allpass_Tap( mLoop[ mLoopSelectSingleNumber ]->getAllpass_Tap() );
		emit Message( 2, QString( "Allpass slave ratio SINGLE : " + QString::number( mLoopSelectSingleNumber, 10 ) ) ); /////////////////////////
		emit Changed_Allpass_Tap_Slider( mLoop[ mLoopSelectSingleNumber ]->getAllpass_Tap_Slider() );
	} else {
		for ( int i=0; i<7; i++) {
			if ( mLoopSelect[ i ] ) {
				mLoop[ i ]->setAllpass_SlaveRatio( val );
	       		} 
	    	}
		mLoop[ 15 ]->setAllpass_SlaveRatio( val );
		// ##############################		
		emit Changed_Allpass_Tap( mLoop[ 15 ]->getAllpass_Tap() );
		emit Message( 2, QString( "Allpass slave ratio MULTI : " + QString::number( 15, 10 ) ) ); /////////////////////////
		emit Changed_Allpass_Tap_Slider( mLoop[ 15 ]->getAllpass_Tap_Slider() );
	}
	
}

void R_TNishimu::setLoopAllpass_Tap_Relative( int val )
{
	emit Message( 2, QString("Loop allpass relative ")); /////////////////////////
	for ( int i=0; i<7; i++) {
		mLoop[ i ]->setAllpass_Tap_Relative( val );
	}
	mLoop[ 15 ]->setAllpass_Tap_Relative( val );
	
	int theSelect;	
	if ( !mLoopSelectMulti ) {
	    	theSelect = mLoopSelectSingleNumber;
	} else {
		theSelect = 15;
	}
		// ##############################		
		emit Changed_Allpass_Tap( mLoop[ theSelect ]->getAllpass_Tap() );
		//emit Changed_Allpass_Tap_Slider( mLoop[ theSelect ]->getAllpass_Tap_Slider() );
}

void R_TNishimu::setLoopAllpass_Level( int val )
{
	emit Message( 2, QString("Loop allpass level ")); /////////////////////////
	if ( !mLoopSelectMulti ) {
		mLoop[ mLoopSelectSingleNumber ]->setAllpass_Level( val );
		// ###################################
		emit Changed_Allpass_Level( mLoop[ mLoopSelectSingleNumber ]->getAllpass_Level() );
		emit Message( 2, QString( "Allpass level SINGLE : " + QString::number( mLoopSelectSingleNumber, 10 ) ) ); /////////////////////////
	} else {
		for ( int i=0; i<7; i++) {
			if ( mLoopSelect[ i ] ) {
				mLoop[ i ]->setAllpass_Level( val );
	       		} 
		}
		mLoop[ 15 ]->setAllpass_Level( val );
		// ###################################
		emit Changed_Allpass_Level( mLoop[ 15 ]->getAllpass_Level() );
		emit Message( 2, QString( "Allpass level MULTI : " + QString::number( 15, 10 ) ) ); /////////////////////////
	}
}

void R_TNishimu::setLoopAllpass_Level_Relative( int val )
{
	for ( int i=0; i<7; i++) {
		mLoop[ i ]->setAllpass_Level_Relative( val );
	}
	mLoop[ 15 ]->setAllpass_Level_Relative( val );
	
	int theSelect;
	if ( !mLoopSelectMulti ) {
	    	theSelect = mLoopSelectSingleNumber;
	} else {
		theSelect = 15;
	}
		// ###################################
		emit Changed_Allpass_Level( mLoop[ theSelect ]->getAllpass_Level() );
		//emit Changed_Allpass_Level_Slider( mLoop[ theSelect ]->getAllpass_Level_Slider() );
		emit Message( 1, QString("Loop allpass level reltive ")); /////////////////////////
}

// /////////////////////////////////////////////////////////////////////////////////  
// Loop Feedback
void R_TNishimu::setLoopFeedback_Tap(int val )
{
	if ( !mLoopSelectMulti ) {
		mLoop[ mLoopSelectSingleNumber ]->setFeedback_Tap( val );
		// ###################################
		emit Changed_Feedback_Tap( mLoop[ mLoopSelectSingleNumber ]->getFeedback_Tap() );
		// ##############################		
		emit Changed_Allpass_Tap( mLoop[ mLoopSelectSingleNumber ]->getAllpass_Tap() );
		emit Changed_Allpass_Tap_Slider( mLoop[ mLoopSelectSingleNumber ]->getAllpass_Tap_Slider() );
		emit Message( 2, QString( "Feedback Tap SINGLE : " + QString::number( mLoopSelectSingleNumber, 10 ) ) ); /////////////////////////
	} else {
		for ( int i=0; i<7; i++) {
			if ( mLoopSelect[ i ] ) {
				mLoop[ i ]->setFeedback_Tap( val );
	       		} 
	    	}
		mLoop[ 15 ]->setFeedback_Tap( val );
		// ###################################
		emit Changed_Feedback_Tap( mLoop[ 15 ]->getFeedback_Tap() );
		// ##############################		
		emit Changed_Allpass_Tap( mLoop[ 15 ]->getAllpass_Tap() );
		emit Changed_Allpass_Tap_Slider( mLoop[ 15 ]->getAllpass_Tap_Slider() );
		emit Message( 2, QString( "Feedback Tap MULTI : " + QString::number( 15, 10 ) ) ); /////////////////////////
	}
}

void R_TNishimu::setLoopFeedback_Tap_Relative( int val )
{
	for ( int i=0; i<7; i++) {
		mLoop[ i ]->setFeedback_Tap_Relative( val );
	}
	mLoop[ 15 ]->setFeedback_Tap_Relative( val );
	
	int num;
	if ( !mLoopSelectMulti ) {
	    	num = mLoopSelectSingleNumber;
	} else {
		num = 15;
	}
	
	emit Changed_Feedback_Tap( mLoop[ num ]->getFeedback_Tap() );
	emit Changed_Allpass_Tap( mLoop[ num ]->getAllpass_Tap() );
		
		//emit Changed_Allpass_Tap_Slider( mLoop[ num ]->getAllpass_Tap_Slider() );
		//emit Changed_Feedback_Tap_Slider( mLoop[ num ]->getFeedback_Tap_Slider() );
		emit Message( 2, QString("Loop feedback tap rerative ")); ///////////////////////////
}

void R_TNishimu::setLoopFeedback_Level(int val )
{
	emit Message( 2, QString("Loop feedback level")); ////////////////////////////
	if ( !mLoopSelectMulti ) {
		mLoop[ mLoopSelectSingleNumber ]->setFeedback_Level( val );
		// ###################################
		emit Changed_Feedback_Level( mLoop[ mLoopSelectSingleNumber ]->getFeedback_Level() );
		//emit Changed_Allpass_Level_Slider( mLoop[ mLoopSelectSingleNumber ]->getAllpass_Level_Slider() );
		emit Message( 2, QString( "Feedback level SINGLE : " + QString::number( mLoopSelectSingleNumber, 10 ) ) ); /////////////////////////
	} else {
	    	for ( int i=0; i<7; i++) {
			if ( mLoopSelect[ i ] ) {
				mLoop[ i ]->setFeedback_Level( val );
	       		} 
	    	}
	   	mLoop[ 15 ]->setFeedback_Level( val );
		emit Changed_Feedback_Level( mLoop[ 15 ]->getFeedback_Level() );
		//emit Changed_Allpass_Level_Slider( mLoop[ 15 ]->getAllpass_Level_Slider() );
		emit Message( 2, QString( "Feedback level MULTI : " + QString::number( 15, 10 ) ) ); /////////////////////////
	}
}

void R_TNishimu::setLoopFeedback_Level_Relative( int val )
{
	emit Message( 2, QString("Loop feedback level relative")); ////////////////////////
	for ( int i=0; i<7; i++) {
		mLoop[ i ]->setFeedback_Level_Relative( val );
	}
		mLoop[ 15 ]->setFeedback_Level_Relative( val );
	int num;
	if ( !mLoopSelectMulti ) {
	    	num = mLoopSelectSingleNumber;
	    } else {
		num = 15;
	    }
		// ###################################
		emit Changed_Feedback_Level( mLoop[ mLoopSelectSingleNumber ]->getFeedback_Level() );
		//emit Changed_Allpass_Level_Slider( mLoop[ mLoopSelectSingleNumber ]->getAllpass_Level_Slider() );
		//emit Changed_Feedback_Level_Slider( mLoop[ mLoopSelectSingleNumber ]->getFeedback_Level_Slider() );
}

// /////////////////////////////////////////////////////////////////////////////////  
// Loop Output
void R_TNishimu::setLoopOutput_Tap( int num, int val )
{
	emit Message( 2, QString("Loop output tap "));
		
	if ( !mLoopSelectMulti ) {
		mLoop[ mLoopSelectSingleNumber ]->setOutput_Tap( num, val );
		emit Message( 2, QString( "Output Tap SINGLE : " + QString::number( mLoopSelectSingleNumber, 10 ) ) ); /////////////////////////
	} else {
	    	for ( int i=0; i<7; i++) {
			if ( mLoopSelect[ i ] ) {
				mLoop[ i ]->setOutput_Tap( num, val );
	       		} 
	   	}
		mLoop[ 15 ]->setOutput_Tap( num, val );
		emit Message( 2, QString( "Output Tap MULTI : " + QString::number( 15, 10 ) ) ); /////////////////////////
	}
}

void R_TNishimu::setLoopOutput_Level( int num, int val )
{
	emit Message( 2, QString("Loop output level ")); /////////////////////////
	if ( !mLoopSelectMulti ) {
		mLoop[ mLoopSelectSingleNumber ]->setOutput_Level( num, val );
		emit Message( 2, QString( "Output level SINGLE : " + QString::number( mLoopSelectSingleNumber, 10 ) ) ); ///////////////////////// 	} else {
	} else {
	    for ( int i=0; i<7; i++) {
		if ( mLoopSelect[ i ] ) {
			mLoop[ i ]->setOutput_Level( num, val );
	       	} 
	    }
			mLoop[ 15 ]->setOutput_Level( num, val );
		emit Message( 2, QString( "Output level MULTI : " + QString::number( 15, 10 ) ) ); 
	 }
}


void R_TNishimu::setLoopOutput_Channel( int num, int val )
{
	emit Message( 2, QString("Loop output channel ")); /////////////////////////
	if ( !mLoopSelectMulti ) {
		mLoop[ mLoopSelectSingleNumber ]->setOutput_Channel( num, val );
		emit Message( 2, QString( "Output channel SINGLE : " + QString::number( mLoopSelectSingleNumber, 10 ) ) ); 
	} else {
	   	 for ( int i=0; i<7; i++) {
			if ( mLoopSelect[ i ] ) {
				mLoop[ i ]->setOutput_Channel( num, val );
	       		} 
		}
		mLoop[ 15 ]->setOutput_Channel( num, val );
		emit Message( 2, QString( "Output level MULTI : " + QString::number( 15, 10 ) ) ); 
	}
}

// ============================================================

void R_TNishimu::setPreEQSelectSingle( int number )
{
	mPreEQSelectMulti = false;
	mPreEQSelectSingleNumber = number;
	EmitChangedPreEQStatus( );
}

void R_TNishimu::setPreEQSelectMulti(  )
{
	mPreEQSelectMulti = true;
	mPreEQSelectSingleNumber = 15;
	EmitChangedPreEQStatus( );
}		
		
void R_TNishimu::EmitChangedPreEQStatus(  )
{
    int num;
    if ( mPreEQSelectMulti ) num = 15;
    else  num = mPreEQSelectSingleNumber;
    
	emit Changed_PreEQ_Freq( mEQ[ mPreEQControlMode ][ num ]->mFreqSent );
	emit Changed_PreEQ_Level( mEQ[ mPreEQControlMode ][ num ]->mLevelSent );
	emit Changed_PreEQ_Q( mEQ[ mPreEQControlMode ][ num ]->mQSent );
	emit Changed_PreEQ_Type( mEQ[ mPreEQControlMode ][ num ]->mType );
			 
	emit Changed_PreEQ_Freq_Slider( mEQ[ mPreEQControlMode ][ num ]->mFreqLog );
	emit Changed_PreEQ_Level_Slider( mEQ[ mPreEQControlMode ][ num ]->mLevel );
	emit Changed_PreEQ_Q_Slider( mEQ[ mPreEQControlMode ][ num ]->mQ );

}

void R_TNishimu::setLoopSelectSingle( int number )
{
	emit Message( 2, QString("Loop select single :  " + QString::number( number, 10 ) ) ); /////////////////////////
	 
	mLoopSelectMulti = false;    
	mLoopSelectSingleNumber = number; 
	EmitChangedLoopStatus(  );
}


void R_TNishimu::setLoopSelectMulti(  )
{
	mLoopSelectMulti = true;
	mLoopSelectSingleNumber = 15;
	EmitChangedLoopStatus( );
}		
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void R_TNishimu::EmitChangedLoopStatus(  )
{ 
	emit Message( 2, QString("Emit changed Loop Status :  " + QString::number( mLoopSelectSingleNumber, 10 ) ) ); /////////////////////////
	//return;
	
	int num;
    if ( mLoopSelectMulti ) num = 15;
    else  num = mLoopSelectSingleNumber;
   
	emit Changed_Allpass_SlaveMode( mLoop[ num ]->mAllpassSlave );
	emit Changed_Allpass_SlaveRatio( mLoop[ num ]->mAllpassSlaveRatio );
	
	//emit Changed_LoopPreDelay_0_Tap( mLoop[ num ]->getLoopPreDelay_Tap( 0 ) );
	//emit Changed_LoopPreDelay_1_Tap( mLoop[ num ]->getLoopPreDelay_Tap( 1 ) );	
	//emit Changed_LoopPreDelay_0_Level( mLoop[ num ]->getLoopPreDelay_Level( 0 ) );
	//emit Changed_LoopPreDelay_1_Level( mLoop[ num ]->getLoopPreDelay_Level( 1 ) );
	//emit Changed_LoopEQ_0_Freq( mLoop[ num ]->getEQ_Freq( 0 ) );
	//emit Changed_LoopEQ_1_Freq( mLoop[ num ]->getEQ_Freq( 1 ) );
	//emit Changed_LoopEQ_0_Level( mLoop[ num ]->getEQ_Level( 0 ) );
	//emit Changed_LoopEQ_1_Level( mLoop[ num ]->getEQ_Level( 1 ) );
	
	emit Changed_LoopPreDelay_0_Tap_Slider( mLoop[ num ]->getLoopPreDelay_Tap_Slider( 0 ) );
	emit Changed_LoopPreDelay_1_Tap_Slider( mLoop[ num ]->getLoopPreDelay_Tap_Slider( 1 ) );	
	emit Changed_LoopPreDelay_0_Level_Slider( mLoop[ num ]->getLoopPreDelay_Level_Slider( 0 ) );
	emit Changed_LoopPreDelay_1_Level_Slider( mLoop[ num ]->getLoopPreDelay_Level_Slider( 1 ) );
	emit Changed_LoopEQ_0_Freq_Slider( mLoop[ num ]->getEQ_Freq_Slider( 0 ) );
	emit Changed_LoopEQ_1_Freq_Slider( mLoop[ num ]->getEQ_Freq_Slider( 1 ) );
	emit Changed_LoopEQ_0_Level_Slider( mLoop[ num ]->getEQ_Level_Slider( 0 ) );
	emit Changed_LoopEQ_1_Level_Slider( mLoop[ num ]->getEQ_Level_Slider( 1 ) );
	emit Changed_LoopEQ_Type( 0, mLoop[ num ]->getEQ_Type( 0 ));
	emit Changed_LoopEQ_Type( 1, mLoop[ num ]->getEQ_Type( 1 ));

	//emit Changed_Allpass_Tap( mLoop[ num ]->getAllpass_Tap() );
	//emit Changed_Allpass_Level( mLoop[ num ]->getAllpass_Level() );
	//emit Changed_Feedback_Tap( mLoop[ num ]->getAllpass_Tap() );
	//emit Changed_Feedback_Level( mLoop[ num ]->getAllpass_Level() );
	
	emit Changed_Allpass_Tap_Slider( mLoop[ num ]->getAllpass_Tap_Slider() );
	emit Changed_Allpass_Level_Slider( mLoop[ num ]->getAllpass_Level_Slider() );
	emit Changed_Feedback_Tap_Slider( mLoop[ num ]->getFeedback_Tap_Slider() );
	emit Changed_Feedback_Level_Slider( mLoop[ num ]->getFeedback_Level_Slider() );
	emit Changed_LoopOutputCh( 0, mLoop[ num ]->getLoopOutputCh( 0 ) );
	emit Changed_LoopOutputCh( 1, mLoop[ num ]->getLoopOutputCh( 1 ) );
	emit Changed_LoopOutputCh( 2, mLoop[ num ]->getLoopOutputCh( 2 ) );
	emit Changed_LoopOutputCh( 3, mLoop[ num ]->getLoopOutputCh( 3 ) );

	emit Changed_LoopOutputLevel( 0, mLoop[ num ]->getLoopOutputLevel( 0 ) );
	emit Changed_LoopOutputLevel( 1, mLoop[ num ]->getLoopOutputLevel( 1 ) );
	emit Changed_LoopOutputLevel( 2, mLoop[ num ]->getLoopOutputLevel( 2 ) );
	emit Changed_LoopOutputLevel( 3, mLoop[ num ]->getLoopOutputLevel( 3 ) );

	emit Changed_LoopOutputDelay( 1, mLoop[ num ]->getLoopOutputDelay( 1 ) );
	emit Changed_LoopOutputDelay( 2, mLoop[ num ]->getLoopOutputDelay( 2 ) );
	emit Changed_LoopOutputDelay( 3, mLoop[ num ]->getLoopOutputDelay( 3 ) );
	
	emit Changed_Transaural_DelayLevel( mTrans->mTap,  mTrans->mLevel );
	emit Changed_FxOff_DirectLevel( mMixer_Effect_Off->mDirectLevel );
	emit Changed_FxOff_EffectLevel( mMixer_Effect_Off->mEffectLevel );
	emit Changed_FxOn_DirectLevel( mMixer_Effect_On->mDirectLevel );
	emit Changed_FxOn_EffectLevel( mMixer_Effect_On->mEffectLevel );
}


	
// /////////////////////////////////////////////
void R_TNishimu::saveAllData( QTextStream* file )
{
	file->writeRawBytes ( (char*)( &mDataVersion ), sizeof( mDataVersion ) );
	
	file->writeRawBytes ( (char*)( &mPreEQControlMode ), sizeof( mPreEQControlMode ) );	
	file->writeRawBytes ( (char*)( &mPreEQSelectSingleNumber ), sizeof( mPreEQSelectSingleNumber ) );
	file->writeRawBytes ( (char*)( &mPreEQSelectMulti ), sizeof( mPreEQSelectMulti ) );
	file->writeRawBytes ( (char*)( &mLoopSelectSingleNumber ), sizeof( mLoopSelectSingleNumber ) );
	file->writeRawBytes ( (char*)( &mLoopSelectMulti ), sizeof( mLoopSelectMulti ) );
	
	emit Message(QString(" Save ALL data ")); ////////////////////////////
	//QString str = "Address:" + QString::number( (long)(&mEQ[ 0 ][ 0 ]), 16 ).upper() + " ";
	//emit Message( 1, QString( str )); 
		
	//mEQ[ 0 ][ 0 ]->saveAllData( file );
	//return;
	{	
		for ( int m = 0; m<3; m++ ) {
			for ( int i = 0; i<16; i++ ) {
				mEQ[ m ][ i]->saveAllData( file );
			}
		}
    	}
	{
		for ( int i = 0; i<16; i++ ) {
			mLoop[ i ]->saveAllData( file );
		}
    	}
	mTrans->saveAllData( file );
	
	{
		for ( int i=0; i<16; i++ ) {
			file->writeRawBytes ( (char*)( &mPreEQSelect[ i ] ), sizeof( bool ) );
   		}
	}
	
	{
		for ( int i=0; i<16; i++ ) {
			file->writeRawBytes ( (char*)( &mLoopSelect[ i ] ), sizeof( bool ) ); 
			emit Message( 1, QString(" Save Loop select : "+ QString::number( i, 10 ) + " = " + QString::number( mLoopSelect[ i ], 10 ))); ////////////////////////////
   		}
	}
	mMixer_Effect_Off->saveAllData( file );
	mMixer_Effect_On->saveAllData( file );
	
//	file->writeRawBytes ( (char*)( & ), sizeof(  ) );
}

void R_TNishimu::loadAllData( QTextStream* file )
{
	emit Message( 1, QString(" Load ALL data ")); ////////////////////////////
	long theVersion;
	file->readRawBytes ( (char*)( &theVersion ), sizeof( theVersion ) );
	if ( theVersion == 30722001 ) {
	} else if (theVersion == 30728001 ) {
	} else {
		emit Message( 0, QString(" Different version : "+ QString::number( mDataVersion, 10 ))); ////////////////////////////
		return;
	}
	
	file->readRawBytes ( (char*)( &mPreEQControlMode ), sizeof( mPreEQControlMode ) );	
	emit Message( 1, QString(" Load EQ Controll mode : "+ QString::number( mPreEQControlMode, 10 ))); ////////////////////////////
	
	file->readRawBytes ( (char*)( &mPreEQSelectSingleNumber ), sizeof( mPreEQSelectSingleNumber ) );
	emit Message( 1, QString(" Load EQ Select single number : "+ QString::number( mPreEQSelectSingleNumber, 10 ))); ////////////////////////////
	
	file->readRawBytes ( (char*)( &mPreEQSelectMulti ), sizeof( mPreEQSelectMulti ) );
	emit Message( 1, QString(" Load EQ Select multi mode : "+ QString::number( mPreEQSelectMulti, 10 ))); ////////////////////////////
	
	file->readRawBytes ( (char*)( &mLoopSelectSingleNumber ), sizeof( mLoopSelectSingleNumber ) );
	emit Message( 1, QString(" Load Loop Select single number : "+ QString::number( mLoopSelectSingleNumber, 10 ))); ////////////////////////////
	
	file->readRawBytes ( (char*)( &mLoopSelectMulti ), sizeof( mLoopSelectMulti ) );
	emit Message( 1, QString(" Load Loop Select mullti mode : "+ QString::number( mLoopSelectMulti, 10 ))); ////////////////////////////
	
	emit Message( 1, QString(" -------------------- ")); ////////////////////////////
	emit Message( 1, QString(" Load ALL data 1 ")); ////////////////////////////
	//QString str = "Address:" + QString::number( (long)(&mEQ[ 0 ][ 0 ]), 16 ).upper() + " ";
	//emit Message( 1, QString( str )); 
	
	//mEQ[ 0 ][ 0 ]->loadAllData( file );
	//return;
	///*
	for ( int m = 0; m<3; m++ ) {
	    emit Message( 1, QString("Load PreEQ Control mode ========= : "+ QString::number( m, 10 ))); ////////////////////////////
		for ( int i = 0; i<16; i++ ) {
			emit Message( 1, QString(" PreEQ number : "+ QString::number( i, 10 ))); ////////////////////////////
			mEQ[ m ][ i ]->loadAllData( file );
		}
	}
	//*/
	{
		emit Message( 1, QString("Load Loop ================================== ")); ////////////////////////////
		for ( int i = 0; i<16; i++ ) {
			emit Message( 1, QString(" Loop number : "+ QString::number( i, 10 ))); ////////////////////////////
			mLoop[ i ]->loadAllData( file );
		}
    	}
	mTrans->loadAllData( file );
	
	////////////////////////////////////////////
		{
		    for ( int i=0; i<16; i++ ) {
			file->readRawBytes ( (char*)( &mPreEQSelect[ i ] ), sizeof( bool ) ); 
			emit Message( 1, QString(" Load PreEQ select : "+ QString::number( i, 10 ) + " = " + QString::number( mPreEQSelect[ i ], 10 ))); ////////////////////////////
		    }
		}	
	
	////////////////////////////////////////////
		{
		    for ( int i=0; i<16; i++ ) {
			file->readRawBytes ( (char*)( &mLoopSelect[ i ] ), sizeof( bool ) );
			emit Message( 1, QString(" Load Loop select : "+ QString::number( i, 10 ) + " = " + QString::number( mLoopSelect[ i ], 10 ))); ////////////////////////////
		    }
		}	
	
	if ( theVersion == 30728001 ) {
	    	mMixer_Effect_Off->loadAllData( file );
		mMixer_Effect_On->loadAllData( file );
	}
	emit Message( 1, QString(" ==================- Load completed -================= ")); ////////////////////////////
	
	//EmitChangedLoopStatus(  );
	//EmitChangedPreEQStatus( );
	sendAllData(  );
	ShowData();
	setPreEQSelectSingle( 0 );
	setLoopSelectSingle( 0 );
}

void R_TNishimu::ShowData()
{
	emit Message( 1, QString(" Show data ")); ////////////////////////////
	switch ( mPreEQControlMode ) {
		case 0:	emit Changed_PreEQ_ControlMode_Both( true ); 
			break;
		case 1:	emit Changed_PreEQ_ControlMode_L( true ); 
			break;
		case 2:	emit Changed_PreEQ_ControlMode_R( true ); 
			break;
	}
 	
	if ( mPreEQSelectMulti ) {
		emit CHanged_PreEQ_SelectMulti( true );
	} else {
		switch ( mPreEQSelectSingleNumber ) {
			case 0:	emit Changed_PreEQ_Select_Single_0( true ); 
				break;
			case 1:	emit Changed_PreEQ_Select_Single_1( true ); 
				break;
			case 8:	emit Changed_PreEQ_Select_Single_8( true ); 
				break;
			case 9:	emit Changed_PreEQ_Select_Single_9( true ); 
				break;
			case 15: emit CHanged_PreEQ_SelectMulti( true );
				 break;
		}
    	}
	
	if ( mLoopSelectMulti ) {
		emit CHanged_Loop_SelectMulti( true );
	} else {
		switch ( mLoopSelectSingleNumber ) {
			case 0:	emit Changed_Loop_Select_Single_0( true ); 
				break;
			case 1:	emit Changed_Loop_Select_Single_1( true ); 
				break;
			case 2:	emit Changed_Loop_Select_Single_2( true ); 
				break;
			case 3:	emit Changed_Loop_Select_Single_3( true ); 
				break;
			case 4:	emit Changed_Loop_Select_Single_4( true ); 
				break;
			case 5:	emit Changed_Loop_Select_Single_5( true ); 
				break;
			case 6:	emit Changed_Loop_Select_Single_6( true ); 
				break;
			case 7:	emit Changed_Loop_Select_Single_7( true ); 
				break;
			case 15: emit CHanged_Loop_SelectMulti( true );
				break;
		}
   	}
	
		emit Changed_PreEQ_Select_0_Status( mPreEQSelect[ 0 ] );
		emit Changed_PreEQ_Select_1_Status( mPreEQSelect[ 1 ] );
		emit Changed_PreEQ_Select_8_Status( mPreEQSelect[ 8 ] );
		emit Changed_PreEQ_Select_9_Status( mPreEQSelect[ 9 ] );
		
		emit Changed_Loop_Select_0_Status( mLoopSelect[ 0 ] );
		emit Changed_Loop_Select_1_Status( mLoopSelect[ 1 ] );
		emit Changed_Loop_Select_2_Status( mLoopSelect[ 2 ] );
		emit Changed_Loop_Select_3_Status( mLoopSelect[ 3 ] );
		emit Changed_Loop_Select_4_Status( mLoopSelect[ 4 ] );
		emit Changed_Loop_Select_5_Status( mLoopSelect[ 5  ] );
		emit Changed_Loop_Select_6_Status( mLoopSelect[ 6 ] );
		emit Changed_Loop_Select_7_Status( mLoopSelect[ 7 ] );
		
}

	
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void R_TNishimu::sendAllData(  )
{
	mEQ[ mPreEQControlMode ][ 0 ]->sendValue();
	mEQ[ mPreEQControlMode ][ 1 ]->sendValue();
	mEQ[ mPreEQControlMode ][ 8 ]->sendValue();
	mEQ[ mPreEQControlMode ][ 9 ]->sendValue();
	
	{
		for ( int i = 0; i<7; i++ ) {
			mLoop[ i ]->sendAllData(  );
		}
    	}
	mTrans->sendValue(  );
}

// /////////////// New #####################################################################################################
// /////////////// Loop PreDelay Pararm
void R_TNishimu::setLoopPreDelay_Param_Raw( uint loop, uint ch, int tap, int level )
{
    if ( loop > 7 ) {
	emit Message(QString(" Error, Loop PreDelay Param, Out of range, Loop : "+ QString::number( loop, 10 )));
	return;
    }
    if ( ch > 1 ) {
	emit Message(QString(" Error, Loop PreDelay Param, Out of range, Ch : "+ QString::number( ch, 10 )));
	return;
    }
    mLoop[ loop ]->setPreDelay_TapLevel( ch, tap, level );
}
// /////////////// Loop EQ Freq and Type
void R_TNishimu::setLoopEQ_FreqType_Raw( uint loop, uint ch, int freq, int type )
{
    if ( loop > 7 ) {
	emit Message(QString(" Error, Loop EQ FreqType, Out of range, Loop : "+ QString::number( loop, 10 )));
	return;
    }
    if ( ch > 1 ) {
	emit Message(QString(" Error, Loop EQ FreqType, Out of range, Ch : "+ QString::number( ch, 10 )));
	return;
    }
    mLoop[ loop ]->setEQ_FreqType( ch, freq, type );
}

// /////////////// Loop EQ Level
void R_TNishimu::setLoopEQ_Level_Raw( uint loop, uint ch, int val )
{
    if ( loop > 7 ) {
	emit Message(QString(" Error, Loop EQ Level, Out of range, Loop : "+ QString::number( loop, 10 )));
	return;
    }
    if ( ch > 1 ) {
	emit Message(QString(" Error, Loop EQ Level, Out of range, Ch : "+ QString::number( ch, 10 )));
	return;
    }
    mLoop[ loop ]->setEQ_Level( ch, val );
}

// /////////////// Loop Feedback and Allpass 
void R_TNishimu::setLoop_Param_Raw( uint loop, int feedback_tap, int feedback_level, int tap_ratio_n, int tap_ratio_d, int allpass_level)
{
    if ( loop > 7 ) {
	emit Message(QString(" Error, Loop Param, Out of range, Loop : "+ QString::number( loop, 10 )));
	return;
    }

    int theAllpass_tap = feedback_tap * tap_ratio_n / tap_ratio_d ;
    mLoop[ loop ]->setAllpass_TapLevel( theAllpass_tap, allpass_level );
    mLoop[ loop ]->setFeedback_TapLevel( feedback_tap, feedback_level );
}

// /////////////// Loop Output Param 
void R_TNishimu::setLoopOut_Param_Raw( uint loop, uint ch, int dest, int delay, int level)
{
    if ( loop > 7 ) {
	emit Message(QString(" Error, Loop Output Param, Out of range, Loop : "+ QString::number( loop, 10 )));
	return;
    }
    
    if ( ch > 1 ) {
	emit Message(QString(" Error, Loop Output Param, Out of range, Ch : "+ QString::number( ch, 10 )));
	return;
    }

    mLoop[ loop ]->setOutput_DestTapLevel( ch, dest, delay, level );
}


// /////////////// Loop Output Switch 
void R_TNishimu::setLoopOutSwitch( uint loop, uint ch, bool sw )
{
    if ( loop > 7 ) {
	emit Message(QString(" Error, Loop Output Switch, Out of range, Loop : "+ QString::number( loop, 10 )));
	return;
    }
    
    if ( ch > 3 ) {
	emit Message(QString(" Error, Loop Output Switch, Out of range, Ch : "+ QString::number( ch, 10 )));
	return;
    }

    mLoop[ loop ]->setOutput_Switch( ch, sw );
}


	
// /////////////////////////////////////////////
void R_TNishimu::saveAllDataText( QTextStream* file )
{
	//file->writeRawBytes ( (char*)( &mDataVersion ), sizeof( mDataVersion ) );
	(*file) << "\nThis file contains T.Nishimura's piano presence processor parameters";
	(*file) << "\nFile Save Version : " <<  mDataVersion;
	
	//file->writeRawBytes ( (char*)( &mPreEQControlMode ), sizeof( mPreEQControlMode ) );
	(*file) << "\nPreEQ control mode (0=Both, 1=Lch, 2=Rch)  : " <<  mPreEQControlMode; 
	
	//file->writeRawBytes ( (char*)( & mPreEQSelectSingleNumber ), sizeof( mPreEQSelectSingleNumber ) );
	(*file) << "\nPreEQ select single number ( 0, 1, 8, 9 ) : " <<  mPreEQSelectSingleNumber;
	
	//file->writeRawBytes ( (char*)( &mPreEQSelectMulti ), sizeof( mPreEQSelectMulti ) );
	(*file) << "\nPreEQ select multi mode ( 0=Off, 1=On ) : " <<  mPreEQSelectMulti;
	
	// file->writeRawBytes ( (char*)( &mLoopSelectSingleNumber ), sizeof( mLoopSelectSingleNumber ) );
	(*file) << "\nLoop select single number ( 0 - 6 ) : " <<  mLoopSelectSingleNumber;
	
	//file->writeRawBytes ( (char*)( &mLoopSelectMulti ), sizeof( mLoopSelectMulti ) );
	(*file) << "\nLoop select multi mode ( 0=Off, 1=On ) : " <<  mLoopSelectMulti;
	
	emit Message( 1, QString(" Save ALL data ")); ////////////////////////////
	//QString str = "Address:" + QString::number( (long)(&mEQ[ 0 ][ 0 ]), 16 ).upper() + " ";
	//emit Message( 1, QString( str )); 
		
	(*file) << "\n ";
	(*file) << "\n------------------------------- Followings are data contents -------------------------- ";
	//mEQ[ 0 ][ 0 ]->saveAllData( file );
	//return;
	{	
			    (*file) << "\n-------------------- PreEQ ----------------";
			    (*file) << "\n PreEQ number : 0 ";
				mEQ[ mPreEQControlMode ][ 0 ]->saveAllDataText( file );
			    (*file) << "\n PreEQ number : 1 ";
				mEQ[ mPreEQControlMode ][ 1 ]->saveAllDataText( file );
			    (*file) << "\n PreEQ number : 8 ";
				mEQ[ mPreEQControlMode ][ 8 ]->saveAllDataText( file );
			    (*file) << "\n PreEQ number : 9 ";
				mEQ[ mPreEQControlMode ][ 9 ]->saveAllDataText( file );
	}
	{
		 (*file) << "\n-------------------- Loop ----------------";
		for ( int i = 0; i<7; i++ ) {
			(*file) << "\n Loop number :  " << i;
			mLoop[ i ]->saveAllDataText( file );
		}
    	}
				
	 (*file) << "\n-------------------- Transaural ----------------";
	mTrans->saveAllDataText( file );
	(*file) << "\n---------------------------------------- End of contents ---------------------------------- ";
	
	{
		(*file) << "\nPreEQ selected in multi mode " ;
		(*file) << "\n     number : " <<  0 << " : " << mPreEQSelect[ 0 ];
		(*file) << "\n     number : " <<  1 << " : " << mPreEQSelect[ 1 ];
		(*file) << "\n     number : " <<  8 << " : " << mPreEQSelect[ 8 ];
		(*file) << "\n     number : " <<  9 << " : " << mPreEQSelect[ 9 ];
	}
	
	{
		(*file) << "\nLoop selected in multi mode " ;
		for ( int i=0; i<7; i++ ) {
			//file->writeRawBytes ( (char*)( &mLoopSelect[ i ] ), sizeof( bool ) ); 
		   	 (*file) << "\n    number " <<  i << " : " << mLoopSelect[ i ];
 			emit Message( 2, QString(" Save Loop select : "+ QString::number( i, 10 ) + " = " + QString::number( mLoopSelect[ i ], 10 ))); ////////////////////////////
   		}
	}
	
//	file->writeRawBytes ( (char*)( & ), sizeof(  ) );
}


// /////////////////////////////////////////////
void R_TNishimu::saveAllDataAsCoef( QTextStream* file )
{
	emit Message( 1, QString(" Save ALL data ")); ////////////////////////////
	//QString str = "Address:" + QString::number( (long)(&mEQ[ 0 ][ 0 ]), 16 ).upper() + " ";
	//emit Message( 1, QString( str )); 
		
	//mEQ[ 0 ][ 0 ]->saveAllData( file );
	//return;
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
	    (*file) << "\nstatic const SDspStCoefDat oku_preEQ_FA[] = {";
			    //(*file) << "\n PreEQ number : 0 ";
				mEQ[ mPreEQControlMode ][ 0 ]->saveAllDataAsCoef( file, &oku_preEQ_FA[0] );
			    //(*file) << "\n PreEQ number : 1 ";
				mEQ[ mPreEQControlMode ][ 1 ]->saveAllDataAsCoef( file, &oku_preEQ_FA[6] );
			(*file) << "\n};";
				
	    (*file) << "\nstatic const SDspStCoefDat oku_preEQ_VE[] = {";
				//(*file) << "\n PreEQ number : 8 ";
				mEQ[ mPreEQControlMode ][ 8 ]->saveAllDataAsCoef( file, &oku_preEQ_VE[0] );
			   // (*file) << "\n PreEQ number : 9 ";
				mEQ[ mPreEQControlMode ][ 9 ]->saveAllDataAsCoef( file, &oku_preEQ_VE[14] );
			(*file) << "\n};";
	}
	{	
			static const char*  oku_EQ_Lsf[] = {
			    "COE_CM000_0",	"0xFFFFFFFF",
			    "COE_CM001_0",	"0xFFFFFFFF",
			    "COE_CM002_0",	"0xFFFFFFFF",
			    
			    "COE_CM010_0",	"0xFFFFFFFF",
			    "COE_CM011_0",	"0xFFFFFFFF",
			    "COE_CM012_0",	"0xFFFFFFFF",
			    
			    "COE_CM020_0",	"0xFFFFFFFF",
			    "COE_CM021_0",	"0xFFFFFFFF",
			    "COE_CM022_0",	"0xFFFFFFFF",
			    
			    "COE_CM030_0",	"0xFFFFFFFF",
			    "COE_CM031_0",	"0xFFFFFFFF",
			    "COE_CM032_0",	"0xFFFFFFFF",
			    
			    "COE_CM040_0",	"0xFFFFFFFF",
			    "COE_CM041_0",	"0xFFFFFFFF",
			    "COE_CM042_0",	"0xFFFFFFFF",
			    
			    "COE_CM050_0",	"0xFFFFFFFF",
			    "COE_CM051_0",	"0xFFFFFFFF",
			    "COE_CM052_0",	"0xFFFFFFFF",
			    
			    "COE_CM060_0",	"0xFFFFFFFF",
			    "COE_CM061_0",	"0xFFFFFFFF",
			    "COE_CM062_0",	"0xFFFFFFFF",
			};
			static const char* oku_EQ_Hsf[] = {
			    "COE_CM003_0",	"0xFFFFFFFF",
			    "COE_CM004_0",	"0xFFFFFFFF",
			    "COE_CM005_0",	"0xFFFFFFFF",
			    
			    "COE_CM013_0",	"0xFFFFFFFF",
			    "COE_CM014_0",	"0xFFFFFFFF",
			    "COE_CM015_0",	"0xFFFFFFFF",
			    
			    "COE_CM023_0",	"0xFFFFFFFF",
			    "COE_CM024_0",	"0xFFFFFFFF",
			    "COE_CM025_0",	"0xFFFFFFFF",
			    
			    "COE_CM033_0",	"0xFFFFFFFF",
			    "COE_CM034_0",	"0xFFFFFFFF",
			    "COE_CM035_0",	"0xFFFFFFFF",
			    
			    "COE_CM043_0",	"0xFFFFFFFF",
			    "COE_CM044_0",	"0xFFFFFFFF",
			    "COE_CM045_0",	"0xFFFFFFFF",
			    
			    "COE_CM053_0",	"0xFFFFFFFF",
			    "COE_CM054_0",	"0xFFFFFFFF",
			    "COE_CM055_0",	"0xFFFFFFFF",
			    
			    "COE_CM063_0",	"0xFFFFFFFF",
			    "COE_CM064_0",	"0xFFFFFFFF",
			    "COE_CM065_0",	"0xFFFFFFFF",
			};			    
			    
			    
		// (*file) << "\n-------------------- Loop ----------------";
	    (*file) << "\nstatic const SDspStCoefDat oku_EQ_Lsf[] = {";
		for ( int i = 0; i<7; i++ ) {
			//(*file) << "\n Loop number :  " << i;
			mLoop[ i ]->saveEQ0DataAsCoef( file,  &oku_EQ_Lsf[i*6]);
		}
			(*file) << "\n};";
	    (*file) << "\nstatic const SDspStCoefDat oku_EQ_Hsf[] = {";
 		for ( int i = 0; i<7; i++ ) {
			//(*file) << "\n Loop number :  " << i;
			mLoop[ i ]->saveEQ1DataAsCoef( file,  &oku_EQ_Hsf[i*6]);
		}
			(*file) << "\n};";
			
			static const char* oku_iram[] = {
			    "IRM_TADMY00_0",	"IRM_TADMY00_1",
			    "IRM_TADMY10_0",	"IRM_TADMY10_1",
			    "IRM_TADMY20_0",	"IRM_TADMY20_1",
			    "IRM_TADMY30_0",	"IRM_TADMY30_1",
			    
			    "IRM_TADMY01_0",	"IRM_TADMY01_1",
			    "IRM_TADMY11_0",	"IRM_TADMY11_1",
			    "IRM_TADMY21_0",	"IRM_TADMY21_1",
			    "IRM_TADMY31_0",	"IRM_TADMY31_1",
			    
			    "IRM_TADMY02_0",	"IRM_TADMY02_1",
			    "IRM_TADMY12_0",	"IRM_TADMY12_1",
			    "IRM_TADMY22_0",	"IRM_TADMY22_1",
			    "IRM_TADMY32_0",	"IRM_TADMY32_1",
			    
			    "IRM_TADMY03_0",	"IRM_TADMY03_1",
			    "IRM_TADMY13_0",	"IRM_TADMY13_1",
			    "IRM_TADMY23_0",	"IRM_TADMY23_1",
			    "IRM_TADMY33_0",	"IRM_TADMY33_1",
			    
			    "IRM_TADMY04_0",	"IRM_TADMY04_1",
			    "IRM_TADMY14_0",	"IRM_TADMY14_1",
			    "IRM_TADMY24_0",	"IRM_TADMY24_1",
			    "IRM_TADMY34_0",	"IRM_TADMY34_1",
			    
			    "IRM_TADMY05_0",	"IRM_TADMY05_1",
			    "IRM_TADMY15_0",	"IRM_TADMY15_1",
			    "IRM_TADMY25_0",	"IRM_TADMY25_1",
			    "IRM_TADMY35_0",	"IRM_TADMY35_1",
			    
			    "IRM_TADMY06_0",	"IRM_TADMY06_1",
			    "IRM_TADMY16_0",	"IRM_TADMY16_1",
			    "IRM_TADMY26_0",	"IRM_TADMY26_1",
			    "IRM_TADMY36_0",	"IRM_TADMY36_1",
			};			    
			    
	    (*file) << "\nstatic const SDspStIramDat oku_iram_Default[] = {";
 		for ( int i = 0; i<7; i++ ) {
			//(*file) << "\n Loop number :  " << i;
			mLoop[ i ]->saveIramDataAsCoef( file,  &oku_iram[i * 8]);
			(*file) << "\n";
		}
			(*file) << "\n};";
			
			static const char* oku_BA[] = {
			    "COE_BA000_0",	"0xFFFFFFFF",
			    "COE_BA001_0",	"0xFFFFFFFF",
			    "COE_BA002_0",	"0xFFFFFFFF",
			    "COE_BA003_0",	"0xFFFFFFFF",
				    
			    "COE_BA010_0",	"0xFFFFFFFF",
			    "COE_BA011_0",	"0xFFFFFFFF",
			    "COE_BA012_0",	"0xFFFFFFFF",
			    "COE_BA013_0",	"0xFFFFFFFF",
				    
			    "COE_BA020_0",	"0xFFFFFFFF",
			    "COE_BA021_0",	"0xFFFFFFFF",
			    "COE_BA022_0",	"0xFFFFFFFF",
			    "COE_BA023_0",	"0xFFFFFFFF",
				    
			    "COE_BA030_0",	"0xFFFFFFFF",
			    "COE_BA031_0",	"0xFFFFFFFF",
			    "COE_BA032_0",	"0xFFFFFFFF",
			    "COE_BA033_0",	"0xFFFFFFFF",
				    
			    "COE_BA040_0",	"0xFFFFFFFF",
			    "COE_BA041_0",	"0xFFFFFFFF",
			    "COE_BA042_0",	"0xFFFFFFFF",
			    "COE_BA043_0",	"0xFFFFFFFF",
				    
			    "COE_BA050_0",	"0xFFFFFFFF",
			    "COE_BA051_0",	"0xFFFFFFFF",
			    "COE_BA052_0",	"0xFFFFFFFF",
			    "COE_BA053_0",	"0xFFFFFFFF",
				    
			    "COE_BA060_0",	"0xFFFFFFFF",
			    "COE_BA061_0",	"0xFFFFFFFF",
			    "COE_BA062_0",	"0xFFFFFFFF",
			    "COE_BA063_0",	"0xFFFFFFFF",
			};
	    (*file) << "\nstatic const SDspStCoefDat oku_Loop_Out[] = {";
 		for ( int i = 0; i<7; i++ ) {
			//(*file) << "\n Loop number :  " << i;
			mLoop[ i ]->saveBADataAsCoef( file,  &oku_BA[i * 8]);
			(*file) << "\n";
		}
			(*file) << "\n};";
			
			static const char* oku_FBK[] = {
			    "COE_CM009_0",	"0xFFFFFFFF",
			    "COE_CM019_0",	"0xFFFFFFFF",
			    "COE_CM029_0",	"0xFFFFFFFF",
			    "COE_CM039_0",	"0xFFFFFFFF",
			    "COE_CM049_0",	"0xFFFFFFFF",
			    "COE_CM059_0",	"0xFFFFFFFF",
			    "COE_CM069_0",	"0xFFFFFFFF",
			};			
	    (*file) << "\nstatic const SDspStCoefDat oku_FeedBack[] = {";
 		for ( int i = 0; i<7; i++ ) {
			//(*file) << "\n Loop number :  " << i;
			mLoop[ i ]->saveFBKDataAsCoef( file,  &oku_FBK[i*2]);
		}
			(*file) << "\n};";
			
			static const char* oku_Coef8[] = {
			    "COE_CM006_0",	"COE_CM008_0",
			    "COE_CM016_0",	"COE_CM018_0",
			    "COE_CM026_0",	"COE_CM028_0",
			    "COE_CM036_0",	"COE_CM038_0",
			    "COE_CM046_0",	"COE_CM048_0",
			    "COE_CM056_0",	"COE_CM058_0",
			    "COE_CM066_0",	"COE_CM068_0",
			};    
	    (*file) << "\nstatic const SDspStCoefDat oku_Coef8[] = {";
 		for ( int i = 0; i<7; i++ ) {
			//(*file) << "\n Loop number :  " << i;
			mLoop[ i ]->saveFBKDataAsCoef( file,  &oku_Coef8[i*2]);
		}
			(*file) << "\n};";
			
			static const char* oku_PD[] = {
			    "COE_PD000_0",		"0xFFFFFFFF",
			    "COE_PD001_0",		"0xFFFFFFFF",
			    "COE_PD002_0",		"0xFFFFFFFF",
			    "COE_PD003_0",		"0xFFFFFFFF",
			    "COE_PD004_0",		"0xFFFFFFFF",
			    "COE_PD005_0",		"0xFFFFFFFF",
			    "COE_PD006_0",		"0xFFFFFFFF",
			    
			    "COE_PD100_0",		"0xFFFFFFFF",
			    "COE_PD101_0",		"0xFFFFFFFF",
			    "COE_PD102_0",		"0xFFFFFFFF",
			    "COE_PD103_0",		"0xFFFFFFFF",
			    "COE_PD104_0",		"0xFFFFFFFF",
			    "COE_PD105_0",		"0xFFFFFFFF",
			    "COE_PD106_0",		"0xFFFFFFFF",
			};
	    (*file) << "\nstatic const SDspStCoefDat oku_PD_Out[] = {";
 		for ( int i = 0; i<7; i++ ) {
			//(*file) << "\n Loop number :  " << i;
			mLoop[ i ]->savePD0DataAsCoef( file,  &oku_PD[i*2]);
		}
		if (gProgram == 0x00) {
 		for ( int i = 0; i<7; i++ ) {
			//(*file) << "\n Loop number :  " << i;
			mLoop[ i ]->savePD1DataAsCoef( file,  &oku_PD[i*2 + 14]);
		}
	    }
			(*file) << "\n};";
			
			}
				
	// (*file) << "\n-------------------- Transaural ----------------";
	mTrans->saveAllDataAsCoef( file );
	//(*file) << "\n---------------------------------------- End of contents ---------------------------------- ";
}

