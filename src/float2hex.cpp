#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void usage(const char* thisProgName)
{
	printf("hex <--> float converter\n");
	printf("usage: %s (float_val | hex_val)\n", thisProgName);
}

int main(int argc, char **argv)
{
	const char* thisProgName = *argv;
	argc--;
	argv++;

	if (argc == 0) {
		usage(thisProgName);
		exit(1);
	}
	const char* valStr = *argv;
	bool isFloat2Hex = false;

	if (strchr(valStr, '.') != 0) {
		isFloat2Hex = true;
	}

	if (isFloat2Hex) {
		double aVal64 = atof(*argv);
		float aVal32 = aVal64;

		unsigned long aHex32Val = *(unsigned long*)(&aVal32);
		unsigned long long aHex64Val = *(unsigned long long*)(&aVal64);

		printf("hex32 val of %g --> 0x%08x\n", aVal32, aHex32Val);
		printf("hex64 val of %g --> 0x%016llx\n", aVal64, aHex64Val);
	} else {
		if (strlen(valStr) > 10) {	// long long
			unsigned long long aHex64Val = strtoull(valStr, 0, 0);

			double aVal64 = *(double*)(&aHex64Val);

			printf("double val of 0x%016llx --> %g\n", aHex64Val, aVal64);
		} else {
			unsigned long aHex32Val = strtoul(valStr, 0, 0);

			float aVal32 = *(float*)(&aHex32Val);

			printf("float val of 0x%08lx --> %g\n", aHex32Val, aVal32);
		}
	}
}
