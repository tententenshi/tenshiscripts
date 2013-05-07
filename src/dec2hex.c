#include 	<stdio.h>
#define	ROUND(x)	(((x) < 0) ? -0.5 : 0.5 )


void main(int argc,char *argv[])
{
	double decNum;
	int	shift;
	unsigned long coefdata;
	
	if(argc!=2){
		fprintf(stderr,"usage: %s <decimal number>\n", argv[0]); exit(0);
	}

	sscanf(argv[1],"%lf",&decNum);

	if ( decNum < 1.0 && decNum >= -1.0){
		coefdata = (unsigned long)( decNum * 2147483648UL + ROUND(decNum));
		shift = 0;
	}else if ( decNum < 2.0 && decNum >= -2.0){
		coefdata = (unsigned long)( decNum / 2 * 2147483648UL + ROUND(decNum));
		shift = 1;
	}else if ( decNum < 4.0 && decNum >= -4.0){
		coefdata = (unsigned long)( decNum / 4 * 2147483648UL + ROUND(decNum));
		shift = 2;
	}else if ( decNum < 16.0 && decNum >= -16.0){
		coefdata = (unsigned long)( decNum / 16 * 2147483648UL + ROUND(decNum));
		shift = 4;
	}else if ( decNum >= 16.0){
		coefdata = 2147483647UL;
		shift = 4;
	}else if ( decNum < -16.0){
		coefdata = 2147483648UL;
		shift = 4;
	}


	if (decNum > 1 || decNum < -1){
		printf("%d => %#x\n", (unsigned long)decNum, (unsigned long)decNum);
	}
	
	if (decNum < 16.0 && decNum >= -16.0){
		printf("(CoefData)=%#010x\t(w/ shift)=%#010x\tshift=%d\n",
		       (long) (decNum*8388608 + ROUND(decNum)), coefdata, shift);
	}
	
}
