/****
*
*	Key# and Frequency Display Tool
*
*				Nov.8 1995  K.Kurahasi
*
*************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


#define K_MIN	0
#define K_88MIN	21	/* Left edge of 88 key		*/
#define K_76MIN	28	/* Left edge of 76 key		*/
#define K_61MIN	36	/* Left edge of 61 key		*/
#define K_C4	60	/* Center C			*/
#define K_A4	69	/* frequency standard point	*/
#define K_61MAX	96	/* Right edge of 61 key		*/
#define K_76MAX	103	/* Right edge of 76 key		*/
#define K_88MAX	108	/* Right edge of 88 key		*/
#define K_MAX	127

#define	F_A4 440.0

#define	SHARP	10

unsigned short	kTbl[]={ 'A',' ','A','#','B',' ','C',' ','C','#','D',' ',
			 'D','#','E',' ','F',' ','F','#','G',' ','G','#' } ;
float	f_a4=F_A4;



main(argc,argv)

	int	argc ;
	char	*argv[] ;
{
	int	note, sharp, step=1, max, min, i, j,num, key ;
	

	num=0;
	sharp=0;
	note=0;

	if(argc>1){

		for(j=1;j<argc;j++){
			if((*argv[j]=='-')&&(*(argv[j]+1)=='t')){
				if(argv[j+1]!=0){
					f_a4=atof(argv[j+1]);
				}
			}
		}
				
		note=(*argv[1]) ;

		if( (j=chk_arg(*(argv[1]+1)))!=0){
			if(j==SHARP){
				sharp=SHARP;
				if((j=chk_arg(*(argv[1]+2)))<SHARP){
					num=j;
				}
			}else{
				num=j;
			}
		}
	}

	min=K_MIN ;
	max=K_MAX ;
	i=0;

	switch(note){
		case '?': usage(); exit(0) ;

		case '-':
			switch(*(argv[1]+1)){
				case 'h':
				case 'H': usage(); exit(0) ;

				case 'K':
				case 'k': if(argc>2){
						get_key(argv[2],&key);
					  }else{
					  	input_key(&key);
						}
					  show_key(key) ;
					  exit(0) ;

				case 'L':
				case 'l': key_map(); exit(0) ;

				case 'T':
				case 't': i=0;	step=1; break ;

				default : min=12*0; max=min+11; step=1; break;
			}
			break;


		case 'C':
		case 'c': i=0 ;	step=12 ;
			  chk_sharp(sharp, &i);
			  chk_num(num, i);
			  break;

		case 'D':
		case 'd': i=2 ;	step=12 ;
			  chk_sharp(sharp, &i);
			  chk_num(num, i);
			  break;

		case 'E':
		case 'e': i=4 ;	step=12 ;
			  chk_sharp(sharp, &i);
			  chk_num(num, i);
			  break;

		case 'F':
		case 'f': i=5 ;	step=12 ;
			  chk_sharp(sharp, &i);
			  chk_num(num, i);
			  break;

		case 'G':
		case 'g': i=7 ;	step=12 ;
			  chk_sharp(sharp, &i);
			  chk_num(num, i);
			  break;

		case 'A':
		case 'a': i=9 ;	step=12 ;
			  chk_sharp(sharp, &i);
			  chk_num(num, i);
			  break;

		case 'B':
		case 'b': i=11;	step=12 ;
			  chk_sharp(sharp, &i);
			  chk_num(num, i);
			  break;

		case '0': min=12*1  ; max=min+11 ; step=1 ; break ;
		case '1': min=12*2  ; max=min+11 ; step=1 ; break ;
		case '2': min=12*3  ; max=min+11 ; step=1 ; break ;
		case '3': min=12*4  ; max=min+11 ; step=1 ; break ;
		case '4': min=12*5  ; max=min+11 ; step=1 ; break ;
		case '5': min=12*6  ; max=min+11 ; step=1 ; break ;
		case '6': min=12*7  ; max=min+11 ; step=1 ; break ;
		case '7': min=12*8  ; max=min+11 ; step=1 ; break ;
		case '8': min=12*9  ; max=min+11 ; step=1 ; break ;
		case '9': min=12*10 ; max=min+11 ; step=1 ; break ;

		default : i=0 ;
			  step=1 ;
			  break ;
	}


	if((note=='6')&&(num==1)){
		min=K_61MIN; max=K_61MAX; step=1;
	}else if((note=='7')&&(num==6)){
		min=K_76MIN; max=K_76MAX; step=1;
	}else if((note=='8')&&(num==8)){
		min=K_88MIN; max=K_88MAX; step=1;
	}

	if(min<K_MIN)	min=K_MIN ;
	if(max>K_MAX)	max=K_MAX ;

	printf("standard pitch (%g Hz)\n", f_a4);

	dline('=') ;

	for(key=min+i;key<=max;key+=step){

		disp(key,step,max) ;

	}
	dline('=') ;
}


int chk_arg(arg)

	int arg;
{
	switch(arg){
		case '-': return(-1);
		case '0': return(0);
		case '1': return(1);
		case '2': return(2);
		case '3': return(3);
		case '4': return(4);
		case '5': return(5);
		case '6': return(6);
		case '7': return(7);
		case '8': return(8);
		case '9': return(9);
		case '#': return(SHARP);
		default : return(0);
	}
}


chk_sharp(sharp, i)

	int	sharp, *i;
{
	if(sharp==SHARP){
		*i+=1 ;
		if(*i>11) *i-=12;
	}
}


chk_num(num, i)

	int	num, i;
{
	int	j;

	if(num!=0){
		j=i+12*(num+1);
		if((j<K_MIN)||(j>K_MAX))
			printf("\nWarning: Out of Range [C-1]~[G9]\n");
	  	dline('=');
		disp(i+12*(num+1),0,0);
	  	dline('=');
	  	exit(0);
	}
}


int xatoi(c)
	char	c;
{
switch(c){
	case '0': return(0);
	case '1': return(1);
	case '2': return(2);
	case '3': return(3);
	case '4': return(4);
	case '5': return(5);
	case '6': return(6);
	case '7': return(7);
	case '8': return(8);
	case '9': return(9);
	case 'A':
	case 'a': return(10);
	case 'B':
	case 'b': return(11);
	case 'C':
	case 'c': return(12);
	case 'D':
	case 'd': return(13);
	case 'E':
	case 'e': return(14);
	case 'F':
	case 'f': return(15);
	default : return(0);
	}
}


int get_key(str,key)

	char	*str;
	int	*key;
{
	if((str[0]=='0')&&(str[1]=='x')){
		if(str[2]==0){
			printf("Error: Out of Range [0-127/0x00-0x7f]\n") ;
			exit(0);
		}
		*key=16*xatoi(str[2])+xatoi(str[3]);
	}else{
		*key=atoi(str) ;
	}
}


input_key(key)

	int	*key;
{
	char    str[8] ;

	printf("Input key#(0-127/0x00-0x7f) : ") ;
	scanf("%s",str) ;
	get_key(str, key);
}

	
show_key(key)

	int	key;
{
	if((key<K_MIN)||(key>K_MAX)){
		printf("Error: Out of Range [0-127/0x00-0x7f]\n") ;
		exit(0) ;
	}
		
	dline('=') ;
	disp(key,0,0) ;
	dline('=') ;
}

	
disp(key,step,max)

	int	key,step,max;
{
	int	i, j ;
	double	f,p ;

	f=f_a4*(p=pow(2.0,(double)(key-K_A4)/12.0)) ;

	i=key/12-1 ;
	j=((key+3)%12)*2 ;

	printf("%c%c%d",kTbl[j],kTbl[j+1],i) ;
	if(i!=-1)	printf(" ");
	printf(": %3d(%02xH) : %10.4f[Hz]",key,key,f) ;

	if(key==K_A4)		printf(" *") ;
	if(key==K_88MIN)	printf(" Left edge of 88 key") ;
	if(key==K_76MIN)	printf(" Left edge of 76 key") ;
	if(key==K_61MIN)	printf(" Left edge of 61 key") ;
	if(key==K_C4)		printf(" Center C") ;
	if(key==K_61MAX)	printf(" Right edge of 61 key") ;
	if(key==K_76MAX)	printf(" Right edge of 76 key") ;
	if(key==K_88MAX)	printf(" Right edge of 88 key") ;

	printf("\n") ;

	if(step!=1) return(0) ;

	if(j==4){
		if(i==3){
			dline('=') ;
		}else{
			if(key!=max)
				dline('-') ;
		}
	}
}



dline(pat)

	int	pat ;
{
	int	i ;
	for(i=0;i<31;i++)
		printf("%c",pat) ;
	printf("\n") ;
}



key_map()
{
printf("\n");
printf("          C0      C1     C2     C3     C4     C5     C6     C7     C8\n");
printf("88 key	  :    |<-:------:------:------:------:------:------:----->|\n");
printf("               A0                      :                           C8\n");
printf("76 key	  :       : |<---:------:------:------:------:------:-->|  :\n");
printf("                    E1                 :                        G7\n");
printf("61 key	  :       :      |<-----:------:------:------:----->|      :\n");
printf("                         C2                                 C7\n");
}



usage()
{
	printf("\n");
	printf("Usage:	scale [c,c#,d,...,a#,b,-,0,1,2,...8,9,61,76,88,-l,-k,-t,-h,?]\n\n") ;
	printf("<ex.>   %% scale c \n");
	printf("        %% scale 4 \n");
	printf("        %% scale c#- -t 442.0 \n");
	printf("        %% scale -k 0x40 \n");
	printf("        %% scale -l \n\n");
	exit(0);
}
      


