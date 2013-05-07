#include <stdio.h>

main ()
{
	unsigned short testS0 = 0x7fff;
	unsigned short testS1 = 0x8000;
	signed long testL0, testL1;
	testL0 = testS0;
	testL1 = testS1;
	printf("testL0 = testS0 -> 0x%x, testL1 = testS1 -> 0x%x\n", testL0, testL1);
	testL0 = (signed long)testS0 << 8;
	testL1 = (signed long)testS1 << 8;
	printf("testL0 = (signed long)testS0 -> 0x%x, testL1 = (singed long)testS1 -> 0x%x\n", testL0, testL1);	
	testL0 = (signed long)(signed short)testS0 << 8;
	testL1 = (signed long)(signed short)testS1 << 8;
	printf("testL0 = (signed long)(signed short)testS0 -> 0x%x, testL1 = (singed long)(signed short)testS1 -> 0x%x\n", testL0, testL1);	
	testL0 = (signed long)((signed short)testS0) << 8;
	testL1 = (signed long)((signed short)testS1) << 8;
	printf("testL0 = (signed long)((signed short)testS0) -> 0x%x, testL1 = (singed long)((signed short)testS1) -> 0x%x\n", testL0, testL1);
}
