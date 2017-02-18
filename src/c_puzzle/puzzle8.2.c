#include <stdio.h>
#define NEG(a)-a
#define weeks(mins)	(days(mins)/7)
#define days(mins)	(hours(mins)/24)
#define hours(mins)	(mins/60)
#define mins(secs)	(secs/60)
#define TAB(c,i,oi,t)	if(c=='\t')\
				for(t=8-(i-oi-1)%8,oi=i; t; t--)\
					putchar(' ')
#define PR(a)		printf(#a "= %d\t", (int)(a))
#define PRINT(a)	PR(a); putchar('\n')

main()
{
	{
		int x=1;
		PRINT( -NEG(x) );	// some preprocessor may generate "--x" (unary decrement)
	}

	{
		PRINT( weeks(10080) );
		PRINT( days(mins(86400)) );
	}

	{
		static char input[] = "\twhich\tif?";
		char c;
		int i, oldi, temp;

		for( oldi= -1,i=0; (c=input[i])!='\0'; i++ )
			if( c<' ' ) TAB(c,i,oldi,temp);
			else putchar(c);	// this else will be paired with "if" in TAB macro
			putchar('\n');
	}
}
