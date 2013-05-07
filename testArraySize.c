#include <stdio.h>

static char testTbl[][3][4] = {
	{
		{0, 1, 2, 3,},
		{4, 5, 6, 7,},
		{8, 9,10,11,},
	}, {
		{0, 1, 2, 3,},
		{4, 5, 6, 7,},
		{8, 9,10,11,},
	}, {
		{0, 1, 2, 3,},
		{4, 5, 6, 7,},
		{8, 9,10,11,},
	}, {
		{0, 1, 2, 3,},
		{4, 5, 6, 7,},
		{8, 9,10,11,},
	}, {
		{0, 1, 2, 3,},
		{4, 5, 6, 7,},
		{8, 9,10,11,},
	},
};

int main(void)
{
	printf("sizeof(testTbl      ) / sizeof(testTbl[0][0][0]) = %d\n", sizeof(testTbl      ) / sizeof(testTbl[0][0][0]));
	printf("sizeof(testTbl[0]   ) / sizeof(testTbl[0][0][0]) = %d\n", sizeof(testTbl[0]   ) / sizeof(testTbl[0][0][0]));
	printf("sizeof(testTbl[0][0]) / sizeof(testTbl[0][0][0]) = %d\n", sizeof(testTbl[0][0]) / sizeof(testTbl[0][0][0]));
	printf("sizeof(testTbl      ) / sizeof(testTbl[0][0]   ) = %d\n", sizeof(testTbl      ) / sizeof(testTbl[0][0]   ));
	printf("sizeof(testTbl      ) / sizeof(testTbl[0]      ) = %d\n", sizeof(testTbl      ) / sizeof(testTbl[0]      ));
}