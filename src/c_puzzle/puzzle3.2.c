#include "defs.h"

main()
{
	int	x, y, z;

	x=y=0;
	while( y<10 ) ++y; x += y;
	PRINT2("%d",x,y);

	x=y=0;
	while( y<10 ) x += ++y;
	PRINT2("%d",x,y);

	y=1;
	while( y<10 ) {
		x = y++; z = ++y;
	}
	PRINT3("%d",x,y,z);

	for( y=1; y<10; y++ ) x=y;
	PRINT2("%d",x,y);

	for( y=1; (x=y)<10; y++ ) ;
	PRINT2("%d",x,y);

	for( x=0,y=1000; y>1; x++,y/=10 )
	PRINT2("%d",x,y);
}
