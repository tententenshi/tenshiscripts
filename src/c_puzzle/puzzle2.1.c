#include <stdio.h>

#define PRINT(format,x)	printf(#x" = "format"\n",x)

int	integer = 5;
char	character = '5';
char	*string = "5";

main()
{
	PRINT("%d",string); PRINT("%d",character); PRINT("%d",integer);
	PRINT("%s",string); PRINT("%c",character); PRINT("%c",integer=53);
	PRINT("%d", ( '5'>5 ));

	{
		int sx = -8;
		unsigned ux = -8;

		PRINT("%o",sx); PRINT("%o",ux);
		PRINT("%o", sx>>3 ); PRINT("%o", ux>>3 );
		PRINT("%d", sx>>3 ); PRINT("%d", ux>>3 );
	}
}
