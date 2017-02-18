#include <stdio.h>

#define PR(x)	printf(#x" = %g\t",(double)(x))
#define NL putchar('\n')
#define PRINT1(x1)	PR(x1); NL
#define PRINT2(x1,x2)	PR(x1); PRINT1(x2)

main()
{
	double d=3.2, x;
	int i=2, y;

	x = (y=d/i)*2; PRINT2(x,y);
	y = (x=d/i)*2; PRINT2(x,y);

	y = d * (x=2.5/d); PRINT1(y);
	x = d * (y =((int)2.9+1.1)/d); PRINT2(x,y);
}
