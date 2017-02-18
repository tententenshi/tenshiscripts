#include <stdio.h>

#define PR(x)	printf(#x" = %.8g\t", (double)x)
#define NL putchar('\n')
#define PRINT4(x1,x2,x3,x4)	PR(x1); PR(x2); PR(x3); PR(x4); NL

main()
{
	double d;
	float f;
	long l;
	int i;

	i = l = f = d = 100/3; PRINT4(i,l,f,d);
	d = f = l = i = 100/3; PRINT4(i,l,f,d);
	i = l = f = d = 100/3.; PRINT4(i,l,f,d);
	d = f = l = i = (double)100/3;
	PRINT4(i,l,f,d);

	i = l = f = d = (double)(100000/3);
	PRINT4(i,l,f,d);

	d = f = l = i = 100000/3; PRINT4(i,l,f,d);
}
