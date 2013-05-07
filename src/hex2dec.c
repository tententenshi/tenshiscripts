#include 	<stdio.h>
#define	ROUND(x)	(((x) < 0) ? -0.5 : 0.5 )


void main(int argc,char *argv[])
{
	int hexNum;

	if(argc!=2){
		fprintf(stderr,"usage: %s <hex number>\n", argv[0]); exit(0);
	}

	sscanf(argv[1],"%lx",&hexNum);

	printf("%#x => %d\tas CoefData => %.9f\n", hexNum, hexNum, (double)hexNum / 8388608.0 );
}
