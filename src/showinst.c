/*
 *==============================================================
 *
 *      gramchk.c
 *
 *              $Author: tnisimu $
 *              $Revision: 1.2 $
 *              $Date: 2003/07/22 23:38:40 $
 *
 *==============================================================
 */
#include        <stdio.h>
#include        <stdlib.h>

#include        <string.h>


#define MAXSTEP		2048
#define MAXBUF          2048
#define MAXDATA         1024
#define ERAMSTEP	768

#define	ERAM_ACCESS_INT	6
#define	ERAM_READ_LATENCY 7
#define	ERAM_WRITE_LATENCY 7
#define	ERAM_OPE_PROBATION 4

enum {
        LINE_CONECT_OFF = 0,
        LINE_CONECT_ON,
};

enum {
        NOP = 0,
        OBJS,
        COEF,
        IRAM,
        ERAM,
        LABL,
	GRAM
};
enum {
	ERAM_NOP,
	ERAM_READ,
	ERAM_WRITE,
};
enum {
	INST_OTHER,
	INST_EM,
	INST_EM_WO_READ,
	INST_WB,
	INST_IRAM_READ,
};
enum {
	FALSE = 0,
	TRUE,
};

unsigned long   prog[MAXSTEP][2];

const char*	inst[] = {
	"acc0 +=  iram               <<? * coef;",
	"acc0  =  iram               <<? * coef;",
	"acc0  = (iram = acc0)       <<? * coef;",
	"acc0  = (iram = acc1)       <<? * coef;",
	"acc1 +=  iram               <<? * coef;",
	"acc1  =  iram               <<? * coef;",
	"acc1  = (iram = acc0)       <<? * coef;",
	"acc1  = (iram = acc1)       <<? * coef;",

	"acc? +=  gram               <<? * coef;",
	"acc?  =  gram               <<? * coef;",
	"                                       ",
	"                                       ",
	"coef_func;",
	"decode;",
	"         gram = acc0;                  ",
	"         gram = acc1;                  ",

	"acc0 += (iram = nov(acc0))  <<? * coef;",
	"acc0  = (iram = nov(acc0))  <<? * coef;",
	"acc0 += (iram = mute(acc0)) <<? * coef;",
	"acc0  = (iram = mute(acc0)) <<? * coef;",
	"acc0  = env(iram)           <<? * coef;",
	"                                       ",
	"acc0 += (iram = acc0)       <<? * coef;",
	"acc1 += (iram = acc1)       <<? * coef;",
	
	"                                       ",
	"                                       ",
	"acc0 += xtri(iram = acc0)   <<? * coef;",
	"acc0  = xtri(iram = acc0)   <<? * coef;",
	"                                       ",
	"                                       ",
	"acc0 +=  tri(iram = acc0)   <<? * coef;",
	"acc0  =  tri(iram = acc0)   <<? * coef;",
};

const char*	decode[] = {
	"         cs0   = acc0;                 ",
	"         cs0   = acc1;                 ",
	"         cs1   = acc0;                 ",
	"         cs1   = acc1;                 ",
	"         cs2   = acc0;                 ",
	"         cs2   = acc1;                 ",
	"         cs3   = acc0;                 ",
	"         cs3   = acc1;                 ",
	"         cs4   = acc0;                 ",
	"         cs4   = acc1;                 ",
	"         cs5   = acc0;                 ",
	"         cs5   = acc1;                 ",
	"                                       ",
	"                                       ",
	"                                       ",
	"                                       ",

	"         mute(0) = acc0;               ",
	"         mute(0) = acc1;               ",
	"         mute(1) = acc0;               ",
	"         mute(1) = acc1;               ",
	"         mute(2) = acc0;               ",
	"         mute(2) = acc1;               ",
	"         mute(3) = acc0;               ",
	"         mute(3) = acc1;               ",
	"         mute(4) = acc0;               ",
	"         mute(4) = acc1;               ",
	"         mute(5) = acc0;               ",
	"         mute(5) = acc1;               ",
	"                                       ",
	"                                       ",
	"                                       ",
	"                                       ",

	"jz    acc0   adrs;                     ",
	"jn    acc0   adrs;                     ",
	"jp    acc0   adrs;                     ",
	"jump  acc0   adrs;                     ",
	"         int0 = acc0;                  ",
	"         int1 = acc0; // zero cross    ",
	"acc0 +=  dbl           <<? * lwr(coef);",
	"         ip = nov(acc0);               ",
	"                                       ",
	"                                       ",
	"         cb = acc0;                    ",
	"         wb = acc0;                    ",
	"acc0 += (iram(0xfc) = em)   <<? * coef;",
	"acc0 += (iram(0xfd) = em)   <<? * coef;",
	"acc0 += (iram(0xfe) = em)   <<? * coef;",
	"acc0 += (iram(0xff) = em)   <<? * coef;",

	"jz    acc0   adrs;                     ",
	"jn    acc0   adrs;                     ",
	"jp    acc0   adrs;                     ",
	"jump  acc0   adrs;                     ",
	"         int0 = acc0;                  ",
	"         int1 = acc0; // zero cross    ",
	"                                       ",
	"         ip = nov(acc0);               ",
	"                                       ",
	"                                       ",
	"         cb = acc0;                    ",
	"         wb = acc0;                    ",
	"acc0  = (iram(0xfc) = em)   <<? * coef;",
	"acc0  = (iram(0xfd) = em)   <<? * coef;",
	"acc0  = (iram(0xfe) = em)   <<? * coef;",
	"acc0  = (iram(0xff) = em)   <<? * coef;",

	"jz    acc1   adrs;                     ",
	"jn    acc1   adrs;                     ",
	"jp    acc1   adrs;                     ",
	"jump  acc1   adrs;                     ",
	"         int0 = acc1;                  ",
	"         int1 = acc1; // zero cross    ",
	"acc1 +=  dbl           <<? * lwr(coef);",
	"         ip = nov(acc1);               ",
	"                                       ",
	"                                       ",
	"         cb = acc1;                    ",
	"         wb = acc1;                    ",
	"acc1 += (iram(0xfc) = em)   <<? * coef;",
	"acc1 += (iram(0xfd) = em)   <<? * coef;",
	"acc1 += (iram(0xfe) = em)   <<? * coef;",
	"acc1 += (iram(0xff) = em)   <<? * coef;",

	"jz    acc1   adrs;                     ",
	"jn    acc1   adrs;                     ",
	"jp    acc1   adrs;                     ",
	"jump  acc1   adrs;                     ",
	"         int0 = acc1;                  ",
	"         int1 = acc1; // zero cross    ",
	"                                       ",
	"         ip = nov(acc1);               ",
	"                                       ",
	"                                       ",
	"         cb = acc1;                    ",
	"         wb = acc1;                    ",
	"acc1  = (iram(0xfc) = em)   <<? * coef;",
	"acc1  = (iram(0xfd) = em)   <<? * coef;",
	"acc1  = (iram(0xfe) = em)   <<? * coef;",
	"acc1  = (iram(0xff) = em)   <<? * coef;",
};

const char* coef_func[] = {
	"acc0  =  iram           <<? * csN     ;",
	"acc0 +=  iram           <<? * csN     ;",
	"acc1  =  iram           <<? * csN     ;",
	"acc1 +=  iram           <<? * csN     ;",
	"acc0  = (iram = acc0)   <<? * csN     ;",
	"acc0 += (iram = acc0)   <<? * csN     ;",
	"acc1  = (iram = acc1)   <<? * csN     ;",
	"acc1 += (iram = acc1)   <<? * csN     ;",
	"acc0  =  iram           <<? * -csN    ;",
	"acc0 +=  iram           <<? * -csN    ;",
	"acc1  =  iram           <<? * -csN    ;",
	"acc1 +=  iram           <<? * -csN    ;",
	"acc0  = (iram = acc0)   <<? * -csN    ;",
	"acc0 += (iram = acc0)   <<? * -csN    ;",
	"acc1  = (iram = acc1)   <<? * -csN    ;",
	"acc1 += (iram = acc1)   <<? * -csN    ;",
	"acc0  =  iram           <<? * (1-csN) ;",
	"acc0 +=  iram           <<? * (1-csN) ;",
	"acc1  =  iram           <<? * (1-csN) ;",
	"acc1 +=  iram           <<? * (1-csN) ;",
	"acc0  = (iram = acc0)   <<? * (1-csN) ;",
	"acc0 += (iram = acc0)   <<? * (1-csN) ;",
	"acc1  = (iram = acc1)   <<? * (1-csN) ;",
	"acc1 += (iram = acc1)   <<? * (1-csN) ;",
	"                                       ",
	"                                       ",
	"                                       ",
	"                                       ",
	"         iram = sqr(acc0);             ",
	"         iram = sqr(acc0);             ",
	"         iram = sqr(acc1);             ",
	"         iram = sqr(acc1);             ",
};

/*--------------------------------------------------------------*/
/* InitData                                                     */
/*--------------------------------------------------------------*/
static void     InitData(void)
{
        int     i;
        
        for (i = 0; i < MAXSTEP; i++) {
                prog[i][0] = 0;
                prog[i][1] = 0;
        }
}

/*--------------------------------------------------------------*/
/* PrintData2                                                   */
/*--------------------------------------------------------------*/
static void     PrintData2(void)
{        
	int	i,j;
	const char*   	currInst;
	char	currCoef[256];
	int	eram_offset;
	int	instType;
	int	eram_count = 0;

	int	pram_inst;
	int	pram_offset;
	int	pram_shift;
	int	pram_coef;

	int	isNop = FALSE;

	for (j = 0; j < 2; j++){
		printf("processor %d\n", j);
		for (i = 0; i < ERAMSTEP; i++){
			isNop = FALSE;
			pram_inst   = (prog[i][j] & 0x007c0000) >> 18;
			pram_offset = (prog[i][j] & 0x0003fc00) >> 10;
			pram_shift  = (prog[i][j] & 0x00000300) >> 8;
			pram_coef   = (prog[i][j] & 0x000000ff);

			switch (pram_inst){
			case 0x0d: // decode
				if(pram_offset >= 0xa0) {
					currInst = decode[pram_offset - 0xa0];
					sprintf(currCoef, "             s=%1d, c=%5.2f(0x%02x)",
						pram_shift, (char)pram_coef / 128.0, pram_coef);
				}
				break;
			case 0x0c:
				currInst = coef_func[pram_coef & 0x1f];
				sprintf(currCoef, "o=%3d(0x%02x), s=%1d, cs=%1d         ",
					pram_offset, pram_offset, pram_shift, ((pram_coef & 0xe0) >> 5));
				break;
			case 0x00:
				if (prog[i][j] & 0x0003ffff) {
					currInst = inst[pram_inst];
					sprintf(currCoef, "o=%3d(0x%02x), s=%1d, c=%5.2f(0x%02x)",
						pram_offset, pram_offset, pram_shift, (char)pram_coef / 128.0, pram_coef);
				break;
				} else {
					isNop = TRUE;
				}
				break;
			default:
				currInst = inst[pram_inst];
				sprintf(currCoef, "o=%3d(0x%02x), s=%1d, c=%5.2f(0x%02x)",
					pram_offset, pram_offset, pram_shift, (char)pram_coef / 128.0, pram_coef);
				break;
			}

			if (eram_count == 0){
				switch ((prog[i][j] & 0x0c000000) >> 26){
				case 0x01:
					instType = ERAM_READ;
					eram_count = 1;
					eram_offset = (((prog[i+1][j] & 0x0f800000) >> 23) +
						       ((prog[i+2][j] & 0x0f800000) >> 18) +
						       ((prog[i+3][j] & 0x0f800000) >> 13) +
						       ((prog[i+4][j] & 0x03800000) >> 8) );
					break;
				case 0x02:
					instType = ERAM_WRITE;
					eram_count = 1;
					eram_offset = (((prog[i+1][j] & 0x0f800000) >> 23) +
						       ((prog[i+2][j] & 0x0f800000) >> 18) +
						       ((prog[i+3][j] & 0x0f800000) >> 13) +
						       ((prog[i+4][j] & 0x03800000) >> 8) );
					break;
				case 0x03:
					switch ((prog[i+4][1] & 0x02000000) >> 25) {
					case 0:
						instType = ERAM_READ;
						eram_count = 1;
						eram_offset = (((prog[i+1][j] & 0x0f800000) >> 23) +
							       ((prog[i+2][j] & 0x0f800000) >> 18) +
							       ((prog[i+3][j] & 0x0f800000) >> 13) +
							       ((prog[i+4][j] & 0x03800000) >> 8) );
						break;
					case 1:
						instType = ERAM_WRITE;
						eram_count = 1;
						eram_offset = (((prog[i+1][j] & 0x0f800000) >> 23) +
							       ((prog[i+2][j] & 0x0f800000) >> 18) +
							       ((prog[i+3][j] & 0x0f800000) >> 13) +
							       ((prog[i+4][j] & 0x03800000) >> 8) );
						break;
					default:
						break;
					}
				default:
					break;
				}
			} else {
				eram_count = (++eram_count) % 5;
			}

			if (isNop) {
				printf(" %2d%4d 0x%08x                                                                          ",
				       j, i, prog[i][j]);
			} else {
				printf(" %2d%4d 0x%08x  %s  %s",
				       j, i, prog[i][j], currInst, currCoef);
			}
			if (eram_count == 1) {
				if (instType == ERAM_READ) {
					printf("  eram_rd = %6d(0x%05x)", eram_offset, eram_offset);
				} else if (instType == ERAM_WRITE) {
					printf("  eram_WR = %6d(0x%05x)", eram_offset, eram_offset);
				}
			}				
			printf("\n");
		}
		printf("\n");
	}

}

/*--------------------------------------------------------------*/
/* Usage                                                        */
/*--------------------------------------------------------------*/
static void     Usage(char *s, int out)
{
        fprintf(stderr,"usage:\n");
        fprintf(stderr,"  %s <obj file name>\n", s);
        exit(out);
}
/*--------------------------------------------------------------*/
/* Error                                                        */
/*--------------------------------------------------------------*/
static void     Error(char *s, FILE* fp1)
{
        fprintf(stderr,"%s",s);
        fclose(fp1);
        exit(1);
}

/*--------------------------------------------------------------*/
/* GetObjData                                                   */
/*--------------------------------------------------------------*/
static int      GetObjData(char* buf)
{
        char            tmp0[10], tmp1[10];
        int             i;
        int             step;
	int		processorNum;
        unsigned long   progDat;
        
	processorNum = atoi(&buf[0]);

//        if (buf[1] != ' ') {
//                return(1);
//        }

        /*----- Get Step Number -----*/
        for (i = 0; i < 3; i++) {
                tmp0[i] = buf[i+2];
        }


//        if (buf[5] != ' ') {
//                return(1);
//        }
        tmp0[3] = '\0';
        if ((step = strtol(tmp0, NULL, 10)) > MAXSTEP) {
                return(1);
        }

        /*----- Get Pram Data -----*/
        for (i = 0; i < 8; i++) {
                tmp1[i] = *(buf + i + 6);
        }
        if (*(buf + 8 + 6) != ';') {
                        return(1);
        }
        tmp1[9] = '\0';
        progDat = strtoul(tmp1, NULL, 16);

        /*----- Write DATA -----*/
        prog[step][processorNum] = progDat;
        
        return(0);
}


/*--------------------------------------------------------------*/
/* CheckData                                                    */
/*--------------------------------------------------------------*/
static unsigned char      CheckData(FILE *fp)
{
        char            lbuf[MAXBUF], lbuf2[MAXBUF];
        int             status = NOP;
        unsigned char   flag = 0;
	int		x;

        unsigned char   flg_conect = LINE_CONECT_OFF;

        while (fgets(lbuf, MAXBUF, fp) != NULL) {

		if(lbuf[strlen(lbuf)-1] == 0x0a) lbuf[strlen(lbuf)-1] = '\0';

		x = strlen( lbuf );
		while( x-- >0 ){if( lbuf[x]=='\t' ) lbuf[x]=' '; }

                if (lbuf[0] == '#') {
                        switch (lbuf[1]) {
                          case 'o': status = OBJS;      break;
                          case 'c': status = COEF;      break;
                          case 'i': status = IRAM;      break;
                          case 'e': status = ERAM;      break;
                          case 'l': status = LABL;      break;
			  case 'g': status = GRAM;	break;
                          default:  status = NOP;       break;
                        }
                } else if ((lbuf[0] == '/') || (lbuf[0] == '\n') || (lbuf[0] == '\0')) {
                        /*--- do nothing ---*/
		} else if ((lbuf[strlen(lbuf)-1] != ';') && flg_conect != LINE_CONECT_ON){
			strcpy(lbuf2, lbuf);
			flg_conect = LINE_CONECT_ON;
		} else if ((lbuf[strlen(lbuf)-1] != ';') && flg_conect == LINE_CONECT_ON){
			while( lbuf[0] == ' ' )	strcpy(&lbuf[0], &lbuf[1]);
			strcat(lbuf2, lbuf);
			strcpy(lbuf, lbuf2);
			flg_conect = LINE_CONECT_ON;
                } else {

			if ((lbuf[strlen(lbuf)-1] == ';') && flg_conect == LINE_CONECT_ON) {
				while( lbuf[0] == ' ' )	strcpy(&lbuf[0], &lbuf[1]);
				strcat(lbuf2, lbuf);
				strcpy(lbuf, lbuf2);
				lbuf2[0] = '\0';
				flg_conect = LINE_CONECT_OFF;
			}

                        switch (status) {
			case OBJS:
                                flag = (1 << OBJS);
                                if (GetObjData(lbuf)) {
                                        return(1);
                                }
                                break;
                        case COEF:
                                break;
                        case IRAM:
                                break;
                        case ERAM:
                                break;
                        case LABL:
                                break;
                        case GRAM:
                                break;
                        default:
                                break;
                        }
                }
        }
        
        if (flag) {
                return(0);
        } else {
                return(flag);
        }
}

/*--------------------------------------------------------------*/
/* main                                                         */
/*--------------------------------------------------------------*/
int     main(int argc, char *argv[])
{
	unsigned char flag;
        FILE    *inpFps;

//		int     i;
        
        /*----- init. data -----*/
        InitData();
        
        /*----- option check -----*/
        fprintf(stderr, "\n");
        if (argc != 2) {
                Usage(argv[0], 1);
        }
        fprintf(stderr, "--- ESP gram check tool ---\n");
        fprintf(stderr, "               Version 0.5\n");
        fprintf(stderr, "file: %s\n", argv[1]);
        
        /*----- Open Files -----*/
        inpFps = fopen(argv[1], "r");
        if (inpFps == NULL) {
                Error("Can not find input file!\n", inpFps);
        }

        /*----- check data -----*/
        if (flag = CheckData(inpFps)) {
		if (flag & 0x01) fprintf(stderr, "OBJS Error\n");
		else if (flag & 0x02) fprintf(stderr, "COEF Error\n");
		else if (flag & 0x08) fprintf(stderr, "ERAM Error\n");
		else  fprintf(stderr, "Other Error\n");
                Error("Invalid input file!\n", inpFps);
        }
        
        /*----- print data -----*/
        PrintData2();

        fclose(inpFps);
        exit(0);
}
