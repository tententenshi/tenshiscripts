#include <stdio.h>
#define FUDGE(k)	k+3.14159
#define PR(a)		printf(#a "= %d\t", (int)(a))
#define PRINT(a)	PR(a); putchar('\n')
#define PRINT2(a,b)	PR(a); PRINT(b)
#define PRINT3(a,b,c)	PR(a); PRINT2(b,c)
#define MAX(a,b)	(a<b ? b : a)

main()
{
	{
		int x=2;
		PRINT( x*FUDGE(2) );
	}

	{
		int ce1;
		for( ce1=0; ce1<=100; ce1+=50 )
			PRINT2( ce1, 9./5*ce1+32 );
	}

	{
		int x=1, y=2;
		PRINT3( MAX(x++,y) ,x,y );
		PRINT3( MAX(x++,y) ,x,y );
	}
}
