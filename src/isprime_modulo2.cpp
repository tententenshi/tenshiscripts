#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>




#define max(a,b)	(((a) > (b)) ? (a) : (b))

static int modulo2(int targetVal, int divisor, int targetValMSB);
static bool isPrime(int targetVal, int targetValLength);
static bool isPrimitive(int targetVal, int targetValLength);

#include <time.h>
extern void init_genrand(unsigned long s);
extern unsigned long genrand_int32(void);


//==========================================================================
//
//==========================================================================
int main(int argc, const char *argv[])
{
#if 0
  if (argc <= 1) {
    printf("usage %s 10011100 (target value as binary)\n", argv[0]);
    exit(1);
  }

  const char* targetValBinary = argv[1];

  int targetValLength = strlen(targetValBinary);
  int targetVal = 0;

  for (int ch = targetValLength - 1, targetBit = 1; ch >= 0; ch--, targetBit <<= 1) {
    if (targetValBinary[ch] == '1') {
      targetVal |= targetBit;
    }
  }

  return isPrime(targetVal, targetValLength);
#else
  if (argc <= 1) {
    printf("usage %s 24 (target bits)\n", argv[0]);
    exit(1);
  }

  int targetValLength = atoi(argv[1]);
  int generatedCount = 0;

#if 0
  int targetRangeStart = (1 << (targetValLength - 1));
  int targetRangeEnd = (1 << targetValLength) - 1;

  for (int targetVal = targetRangeStart; targetVal <= targetRangeEnd; targetVal++) {
#endif
#if 0
  time_t timer;
  time(&timer);
  init_genrand(timer);

  int targetRangeEnd = ((1 << targetValLength - 15) - 1);
  while (1) {
    if (generatedCount > 0x100) { exit(0); }
    int targetVal = (genrand_int32() & targetRangeEnd) | 0x1f46b << (targetValLength - 16);

//    printf("current target: 0x%x\n", targetVal);
#endif
#if 1
   time_t timer;
   time(&timer);
   init_genrand(timer);

  int targetRangeEnd = (1 << targetValLength) - 1;
  while (1) {
    if (generatedCount > 0x100) { exit(0); }
    int targetVal = genrand_int32() & targetRangeEnd;
    if (!(targetVal & (1 << (targetValLength - 1)))) { continue; }

#endif
//    printf("current targetVal: 0x%x\n", targetVal);
    if (isPrime(targetVal, targetValLength)) {
//      printf("targetVal 0x%x is prime ", targetVal);

      if (isPrimitive(targetVal, targetValLength)) {
	generatedCount++;
	printf("targetVal 0x%x is primitive  ", targetVal);

	for (int bit = targetValLength - 1; bit >= 1; bit--) {
	  int mask = 1 << bit;
	  if (targetVal & mask) {
	    if (bit != 0) {
	      printf("x^%d + ", bit);
	    }
	  }
	}
	printf("1\n");
      }
    }
  }

  return 0;
#endif
}


//==========================================================================
//
//==========================================================================
static bool isPrimitive(int divisor, int targetValLength)
{
  int MSB_val = (1 << (targetValLength - 1));
  int initial_val = 1;
//  int initial_val = MSB_val;
  int dividend = initial_val;
  int cur_val;

#if 1	// Fibonacci rotate right
  int* bit_list;
  bit_list = new int[targetValLength];
  int bit_count = 0;
  for (int bit = 0; bit < targetValLength; bit++) {
    if (divisor & (1 << bit)) {
      bit_list[bit_count++] = bit;
    }
  }
#endif


  // for inspect freqency resoponse
  double sum_lowest_bin1_R = 0;
  double sum_lowest_bin1_I = 0;
  double sum_lowest_bin2_R = 0;
  double sum_lowest_bin2_I = 0;
  double sum_lowest_bin3_R = 0;
  double sum_lowest_bin3_I = 0;
  double sum_lowest_bin4_R = 0;
  double sum_lowest_bin4_I = 0;

  double sum_highest_bin1_R = 0;
  double sum_highest_bin1_I = 0;
  double sum_highest_bin2_R = 0;
  double sum_highest_bin2_I = 0;
  double sum_highest_bin3_R = 0;
  double sum_highest_bin3_I = 0;
  double sum_highest_bin4_R = 0;
  double sum_highest_bin4_I = 0;

  for (int i = 0; i < MSB_val - 2; i++) {
#if 0	// Galois rotate left
    bool isMSB = dividend & MSB_val;
    if (isMSB) {
      dividend ^= divisor;
    }

    cur_val = (dividend > MSB_val / 2) ? (dividend - MSB_val) : dividend;

    dividend <<= 1;
    dividend &= (MSB_val * 2 - 1);
#endif
#if 0	// Galois rotate right
    bool isLSB = dividend & 1;
    if (isLSB) {
      dividend ^= divisor;
    }
    dividend >>= 1;

    cur_val = (dividend > MSB_val / 2) ? (dividend - MSB_val) : dividend;
#endif
#if 1	// Fibonacci rotate left
    int isLSB = 0;
    for (int index = 1; index < bit_count; index++) {
      int bit = 1 << bit_list[index];
      if (dividend & bit) {
	isLSB ^= 1;
      }
    }

    if (isLSB & 1) {
      dividend |= 1;
    }
    dividend &= (MSB_val - 1);

    cur_val = (dividend > MSB_val / 2) ? (dividend - MSB_val) : dividend;

    dividend <<= 1;
#endif
#if 0	// Fibonacci rotate right
    int isMSB = 0;

    for (int index = 0; index < bit_count - 1; index++) {
      int bit = 1 << bit_list[index];
      if (dividend & bit) {
	isMSB ^= 1;
      }
    }
    if (isMSB & 1) {
      dividend |= MSB_val;
    }
    dividend >>= 1;

    cur_val = (dividend > MSB_val / 2) ? (dividend - MSB_val) : dividend;
#endif


//    printf("0x%08x\n", cur_val);

    if (i < 44100) {
    sum_lowest_bin1_R += cur_val * sin(    2 * M_PI * i / 2048);
    sum_lowest_bin1_I += cur_val * cos(    2 * M_PI * i / 2048);
    sum_lowest_bin2_R += cur_val * sin(2 * 2 * M_PI * i / 2048);
    sum_lowest_bin2_I += cur_val * cos(2 * 2 * M_PI * i / 2048);
    sum_lowest_bin3_R += cur_val * sin(3 * 2 * M_PI * i / 2048);
    sum_lowest_bin3_I += cur_val * cos(3 * 2 * M_PI * i / 2048);
    sum_lowest_bin4_R += cur_val * sin(4 * 2 * M_PI * i / 2048);
    sum_lowest_bin4_I += cur_val * cos(4 * 2 * M_PI * i / 2048);

    sum_highest_bin1_R += cur_val * sin(1021 * 2 * M_PI * i / 2048);
    sum_highest_bin1_I += cur_val * cos(1021 * 2 * M_PI * i / 2048);
    sum_highest_bin2_R += cur_val * sin(1022 * 2 * M_PI * i / 2048);
    sum_highest_bin2_I += cur_val * cos(1022 * 2 * M_PI * i / 2048);
    sum_highest_bin3_R += cur_val * sin(1023 * 2 * M_PI * i / 2048);
    sum_highest_bin3_I += cur_val * cos(1023 * 2 * M_PI * i / 2048);
    sum_highest_bin4_R += cur_val * sin(1024 * 2 * M_PI * i / 2048);
    sum_highest_bin4_I += cur_val * cos(1024 * 2 * M_PI * i / 2048);
    }

    if (dividend == initial_val) {
//      printf("division of 0x%x has shorter rotation (0x%x cycle) with fraction 0x%x\n", divisor, i, dividend);
      return false;
    }
  }

//    printf("0x%08x\n", cur_val >> 1);

#if 1	// Fibonacci rotate right
  delete [] bit_list;
#endif

#if 0
  double ave_lowest_bin1 = sqrt(sum_lowest_bin1_R * sum_lowest_bin1_R + sum_lowest_bin1_I * sum_lowest_bin1_I) / (MSB_val - 1);
  double ave_lowest_bin2 = sqrt(sum_lowest_bin2_R * sum_lowest_bin2_R + sum_lowest_bin2_I * sum_lowest_bin2_I) / (MSB_val - 1);
  double ave_lowest_bin3 = sqrt(sum_lowest_bin3_R * sum_lowest_bin3_R + sum_lowest_bin3_I * sum_lowest_bin3_I) / (MSB_val - 1);
  double ave_lowest_bin4 = sqrt(sum_lowest_bin4_R * sum_lowest_bin4_R + sum_lowest_bin4_I * sum_lowest_bin4_I) / (MSB_val - 1);

  double ave_highest_bin1 = sqrt(sum_highest_bin1_R * sum_highest_bin1_R + sum_highest_bin1_I * sum_highest_bin1_I) / (MSB_val - 1);
  double ave_highest_bin2 = sqrt(sum_highest_bin2_R * sum_highest_bin2_R + sum_highest_bin2_I * sum_highest_bin2_I) / (MSB_val - 1);
  double ave_highest_bin3 = sqrt(sum_highest_bin3_R * sum_highest_bin3_R + sum_highest_bin3_I * sum_highest_bin3_I) / (MSB_val - 1);
  double ave_highest_bin4 = sqrt(sum_highest_bin4_R * sum_highest_bin4_R + sum_highest_bin4_I * sum_highest_bin4_I) / (MSB_val - 1);
#else
  double ave_lowest_bin1 = sqrt(sum_lowest_bin1_R * sum_lowest_bin1_R + sum_lowest_bin1_I * sum_lowest_bin1_I) / 44100;
  double ave_lowest_bin2 = sqrt(sum_lowest_bin2_R * sum_lowest_bin2_R + sum_lowest_bin2_I * sum_lowest_bin2_I) / 44100;
  double ave_lowest_bin3 = sqrt(sum_lowest_bin3_R * sum_lowest_bin3_R + sum_lowest_bin3_I * sum_lowest_bin3_I) / 44100;
  double ave_lowest_bin4 = sqrt(sum_lowest_bin4_R * sum_lowest_bin4_R + sum_lowest_bin4_I * sum_lowest_bin4_I) / 44100;

  double ave_highest_bin1 = sqrt(sum_highest_bin1_R * sum_highest_bin1_R + sum_highest_bin1_I * sum_highest_bin1_I) / 44100;
  double ave_highest_bin2 = sqrt(sum_highest_bin2_R * sum_highest_bin2_R + sum_highest_bin2_I * sum_highest_bin2_I) / 44100;
  double ave_highest_bin3 = sqrt(sum_highest_bin3_R * sum_highest_bin3_R + sum_highest_bin3_I * sum_highest_bin3_I) / 44100;
  double ave_highest_bin4 = sqrt(sum_highest_bin4_R * sum_highest_bin4_R + sum_highest_bin4_I * sum_highest_bin4_I) / 44100;
#endif

  double ave_lowest_bin1_dB = 20*log10(fabs(ave_lowest_bin1) / 0x8000);
  double ave_lowest_bin2_dB = 20*log10(fabs(ave_lowest_bin2) / 0x8000);
  double ave_lowest_bin3_dB = 20*log10(fabs(ave_lowest_bin3) / 0x8000);
  double ave_lowest_bin4_dB = 20*log10(fabs(ave_lowest_bin4) / 0x8000);

  double ave_highest_bin1_dB = 20*log10(fabs(ave_highest_bin1) / 0x8000);
  double ave_highest_bin2_dB = 20*log10(fabs(ave_highest_bin2) / 0x8000);
  double ave_highest_bin3_dB = 20*log10(fabs(ave_highest_bin3) / 0x8000);
  double ave_highest_bin4_dB = 20*log10(fabs(ave_highest_bin4) / 0x8000);

  double max_lowest_bin = max(max(ave_lowest_bin1_dB, ave_lowest_bin2_dB), max(ave_lowest_bin3_dB, ave_lowest_bin4_dB));
  double max_highest_bin = max(max(ave_highest_bin1_dB, ave_highest_bin2_dB), max(ave_highest_bin3_dB, ave_highest_bin4_dB));

  printf("low: %gdB, high %gdB (delta: %gdB)\t", max_lowest_bin, max_highest_bin, max_highest_bin - max_lowest_bin);

  return true;
}


//==========================================================================
//
//==========================================================================
static bool isPrime(int targetVal, int targetValLength)
{
  int targetValMSB = (1 << (targetValLength - 1));

//  printf("targetVal = 0x%x\n", targetVal);

  if ((targetVal & 1) == 0) {
//    printf("targetVal 0x%x has no constant term, so this targeVal is not prime\n", targetVal);
    return 0;
  }

  int inspect_bit_max = (targetValLength + 1) / 2;

  for (int divisor = 3; divisor < (1 << inspect_bit_max); divisor += 2) {
    if (modulo2(targetVal, divisor, targetValMSB) == 0) {
//      printf("targetVal(0x%x) is divided by 0x%x\n", targetVal, divisor);
      return 0;
    }
  }

//  printf("targetVal (0x%x) is prime\n", targetVal);
  return 1;
}


//==========================================================================
//
//==========================================================================
static int modulo2(int targetVal, int divisor, int targetValMSB)
{
  int workVal = targetVal;
  int workDivisor = divisor;
  int workValMSB = targetValMSB;
  int curBit = 1;
  while (workDivisor < workValMSB * 2) {
#if 0
    if (workVal & curBit) {
      workVal ^= workDivisor;
    }
    workDivisor <<= 1;
    curBit <<= 1;
#else
    if (workVal & 1) {
      workVal ^= workDivisor;
    }
    workVal >>= 1;
    workValMSB >>= 1;
#endif
  }
//  printf("0x%x mod 0x%x is 0x%x\n", targetVal, divisor, workVal);
  return workVal;
}
