#include <stdio.h>
#include <stdlib.h>
#include <string.h>


enum eMode {
	eModeNone,
	eModeTr1,
	eModeTr2,
	eModeRoff,
};


void usage(const char* cmd)
{
	printf("usage: %s [-option] multiplier(hex value) multiplicand(hex value) [shift]\n", cmd);
	printf("\tcf. %s 0xfffe1234 0x7fdb\n", cmd);
	printf("\n");
	printf("\toptions: -tr1    1's complement\n");
	printf("\toptions: -tr2    2's complement\n");
	printf("\toptions: -roff   round off\n");
}


bool IsPositive(const char* mul, int len)
{
	char tmpStr[2] = {mul[0], '\0'};
	bool isNormalizedLength = (len == 4) || (len == 8) || (len == 16);
	return (!isNormalizedLength || (strtoul(tmpStr, NULL, 16) < 8));
}

bool ParseHexValue(const char* hexTxt, int byte, int len, unsigned short* resArray)
{
	bool isPositive = IsPositive(hexTxt, len);
	for (int i = 0, carry = 1; i < byte; i++) {
		char tmpstr[5];
		int startChar = len - (i + 1) * 4;
		int numChar = (startChar <= 0) ? (4 + startChar) : 4;
		startChar = (startChar < 0) ? 0 : startChar;
		strncpy(tmpstr, &hexTxt[startChar], numChar);
		tmpstr[numChar] = '\0';

		if (isPositive) {
			resArray[i] = strtoul(tmpstr, NULL, 16);
		} else {
			unsigned long tmpVal = (~strtoul(tmpstr, NULL, 16) & ((1 << (numChar * 4)) - 1)) + carry;
			carry = (tmpVal & 0x10000) ? 1 : 0;
			resArray[i] = tmpVal & 0xffff;
		}

//		printf("resArray[%d] = 0x%04x, carry %d\n", i, resArray[i], carry);
	}
	return isPositive;
}

void mpy(const unsigned short* mul1array, int mul1byte,
	 const unsigned short* mul2array, int mul2byte,
	 unsigned short* resultarray,     int resultbyte)
{
	for (int i = 0; i < resultbyte; i++) { resultarray[i] = 0; }
	for (int i = 0; i <= (mul1byte + mul2byte - 2); i++) {
		int mul1Ptr = (i < mul2byte) ? 0 : i - mul2byte + 1;
		int mul2Ptr = i - mul1Ptr;

		unsigned long tmpSum = resultarray[i];
		if (i + 1 < resultbyte) {
			tmpSum += resultarray[i + 1] << 16;
		}
		while ((mul1Ptr < mul1byte) && (mul2Ptr >= 0)) {
			unsigned long tmpMpy = mul1array[mul1Ptr] * mul2array[mul2Ptr];
			if ((tmpSum + tmpMpy) < tmpSum) {	// overflow
				if (i + 2 < resultbyte) {
					resultarray[i + 2]++;	// carry
				} else {
					printf("error!\n");
				}
			}
			tmpSum += tmpMpy;
			mul1Ptr++;
			mul2Ptr--;
		}
		resultarray[i] = tmpSum & 0x0000ffff;
		unsigned short upper = (tmpSum & 0xffff0000) >> 16;
		if (i + 1 < resultbyte) {
			resultarray[i + 1] = upper;
		} else {
			if (upper != 0) {
				printf("error!!\n");
			}
		}
	}
}

void InvertSign(unsigned short* resultarray, int resultbyte, int resultlen)
{
	for (int i = 0, carry = 1; i < resultbyte; i++) {
		int numChar = (resultlen - (i + 1) * 4 <= 0) ? (4 + resultlen - (i + 1) * 4) : 4;

		unsigned long tmpVal = (~resultarray[i] & ((1 << (numChar * 4)) - 1)) + carry;
		carry = (tmpVal & 0x10000) ? 1 : 0;
		resultarray[i] = tmpVal & 0xffff;
	}
}

void ShiftLeft(unsigned short* resultarray, int resultbyte, int shiftbit)
{
	for (int i = 0, carry = 0; i < resultbyte; i++) {
		unsigned long tmpVal = (resultarray[i] << shiftbit) + carry;
		carry = (tmpVal & 0xffff0000) >> 16;
		resultarray[i] = tmpVal & 0xffff;
	}
}

int Rounding(eMode mode, const unsigned short* resultarray, bool isResultPositive, int fractionbyte)
{
	switch (mode) {
	default:
	case eModeNone:
	case eModeTr2:
		return 0;
	case eModeTr1:
		if (!isResultPositive) {
#if 0
			for (int i = 0; i < fractionbyte; i++) {
				if (resultarray[i]) return +1;
			}
#else
			return 1;
#endif
		}
		return 0;
	case eModeRoff:
//		if (resultarray[fractionbyte - 1] & 0x8000) {
		if (resultarray[fractionbyte - 1] & 0x0800) {
			return 1;
		} else {
			return 0;
		}
	}		
}

void PrintResult(const unsigned short* resultarray, int resultbyte, int resultlen, int fractionbyte, int roundval)
{
	printf("0x ");
	printf("%0*x ", resultlen - (resultbyte - 1) * 4, resultarray[resultbyte - 1]);
	for (int i = resultbyte - 2; i >= fractionbyte; i--) {
		printf("%04x ", resultarray[i]);
	}
	if (fractionbyte > 0) {
		printf("| ");
		for (int i = fractionbyte - 1; i >= 0; i--) {
			printf("%04x ", resultarray[i]);
		}
	}
	if (roundval != 0) {
		printf(" (%+d)", roundval);
	}
	printf("\n");
}

int main (int argc, const char *argv[])
{
	const char* cmd = argv[0];
	argv++, argc--;

	eMode mode = eModeNone;
	if ((argc > 1) && ((*argv)[0] == '-')) {	// option exists
		const char* option = *argv;
		argv++, argc--;

		if (strstr(option, "tr1")) {
			mode = eModeTr1;
		} else if (strstr(option, "tr2")) {
			mode = eModeTr2;
		} else if (strstr(option, "roff")) {
			mode = eModeRoff;
		} else {
			usage(cmd);
			exit(1);
		}
	}	

	if ((argc != 2) && (argc != 3)) {
		usage(cmd);
		exit(1);
	}

	const char* mul1 = *argv++;
	const char* mul2 = *argv++;
	int shift = 0;
	if (argc == 3) {
		shift = atoi(*argv++);
	}

	if (strncmp(mul1, "0x", 2) != 0 && strncmp(mul1, "0X", 2) != 0) {
		usage(cmd);
		exit(1);
	}
	if (strncmp(mul2, "0x", 2) != 0 && strncmp(mul2, "0X", 2) != 0) {
		usage(cmd);
		exit(1);
	}

	mul1 += 2;	// skip 0x
	mul2 += 2;
	int mul1len = strlen(mul1);
	int mul2len = strlen(mul2);

	int mul1byte = (mul1len + 3) / 4;	// count as short
	int mul2byte = (mul2len + 3) / 4;	// count as short

	unsigned short* mul1array = new unsigned short [mul1byte];
	unsigned short* mul2array = new unsigned short [mul2byte];

	int resultlen = mul1len + mul2len;
	int resultbyte = (resultlen + 3) / 4;	// count as short
	unsigned short* resultarray = new unsigned short [resultbyte];

	bool isMul1Positive = ParseHexValue(mul1, mul1byte, mul1len, mul1array);
	bool isMul2Positive = ParseHexValue(mul2, mul2byte, mul2len, mul2array);

	mpy(mul1array, mul1byte, mul2array, mul2byte, resultarray, resultbyte);

	bool isResultPositive = (isMul1Positive && isMul2Positive) || (!isMul1Positive && !isMul2Positive);

	int fractionbyte = 0;
	int shiftbit = shift;
	if (mode != eModeNone) {
		fractionbyte = mul2byte;
		shiftbit += (mul2byte * 4 - mul2len) * 4 + 1;
		resultlen += (shiftbit + 2) / 4;
	} else {
		resultlen += (shiftbit + 3) / 4;
	}

	if (resultbyte != (resultlen + 3) / 4) {
		int resultbyte_old = resultbyte;
		resultbyte = (resultlen + 3) / 4;
		unsigned short* resultarray_old = resultarray;
		resultarray = new unsigned short [resultbyte];
		for (int i = 0; i < resultbyte_old; i++) {
			resultarray[i] = resultarray_old[i];	// copy
		}
		delete [] resultarray_old;
	}
	ShiftLeft(resultarray, resultbyte, shiftbit);

	if (!isResultPositive) {
		InvertSign(resultarray, resultbyte, resultlen);
	}

	int roundval = Rounding(mode, resultarray, isResultPositive, fractionbyte);

	PrintResult(resultarray, resultbyte, resultlen, fractionbyte, roundval);
}
