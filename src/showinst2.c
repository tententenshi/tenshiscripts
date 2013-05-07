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

const int shiftValue[] = {
	0,
	1,
	2,
	4,
};

unsigned long   prog[MAXSTEP][2];

typedef struct _sShowInst {
	const char* inst;
	int	coefType; 	// 0 0 0 0 0 0 0 0 
	                        //     | | | | | +-- coef
	                        //     | | | | +---- cs
	                        //     | | | +------ shift
	                        //     | | +-------- acc
	                        //     | +---------- offset
	                        //     +------------ adrs
} sShowInst;

const sShowInst	inst[] = {
	{"acc0 +=  iram(0x%02x)               <<%1d *     0x%02x ;", 0x15,},
	{"acc0  =  iram(0x%02x)               <<%1d *     0x%02x ;", 0x15,},
	{"acc0  = (iram(0x%02x) = acc0)       <<%1d *     0x%02x ;", 0x15,},
	{"acc0  = (iram(0x%02x) = acc1)       <<%1d *     0x%02x ;", 0x15,},
	{"acc1 +=  iram(0x%02x)               <<%1d *     0x%02x ;", 0x15,},
	{"acc1  =  iram(0x%02x)               <<%1d *     0x%02x ;", 0x15,},
	{"acc1  = (iram(0x%02x) = acc0)       <<%1d *     0x%02x ;", 0x15,},
	{"acc1  = (iram(0x%02x) = acc1)       <<%1d *     0x%02x ;", 0x15,},

	{"acc%1d +=  gram(0x%02x)               <<%1d *     0x%02x ;", 0x19,},
	{"acc%1d  =  gram(0x%02x)               <<%1d *     0x%02x ;", 0x19,},
	{"                                                  ", 0x00,},
	{"                                                  ", 0x00,},
	{"coef_func;", 0x00,},
	{"decode;", 0x00,},
	{"         gram(0x%02x) = acc0;                       ", 0x10,},
	{"         gram(0x%02x) = acc1;                       ", 0x10,},

	{"acc0 += (iram(0x%02x) = nov(acc0))  <<%1d *     0x%02x ;", 0x15,},
	{"acc0  = (iram(0x%02x) = nov(acc0))  <<%1d *     0x%02x ;", 0x15,},
	{"acc0 += (iram(0x%02x) = mute(acc0)) <<%1d *     0x%02x ;", 0x15,},
	{"acc0  = (iram(0x%02x) = mute(acc0)) <<%1d *     0x%02x ;", 0x15,},
	{"acc0  = env(iram(0x%02x))           <<%1d *     0x%02x ;", 0x15,},
	{"                                                  ", 0x00,},
	{"acc0 += (iram(0x%02x) = acc0)       <<%1d *     0x%02x ;", 0x15,},
	{"acc1 += (iram(0x%02x) = acc1)       <<%1d *     0x%02x ;", 0x15,},
	
	{"                                                  ", 0x00,},
	{"                                                  ", 0x00,},
	{"acc0 += xtri(iram(0x%02x) = acc0)   <<%1d *     0x%02x ;", 0x15,},
	{"acc0  = xtri(iram(0x%02x) = acc0)   <<%1d *     0x%02x ;", 0x15,},
	{"                                                  ", 0x00,},
	{"                                                  ", 0x00,},
	{"acc0 +=  tri(iram(0x%02x) = acc0)   <<%1d *     0x%02x ;", 0x15,},
	{"acc0  =  tri(iram(0x%02x) = acc0)   <<%1d *     0x%02x ;", 0x15,},
};

const sShowInst	decode[] = {
	{"         cs0   = acc0;                            ", 0x00,},
	{"         cs0   = acc1;                            ", 0x00,},
	{"         cs1   = acc0;                            ", 0x00,},
	{"         cs1   = acc1;                            ", 0x00,},
	{"         cs2   = acc0;                            ", 0x00,},
	{"         cs2   = acc1;                            ", 0x00,},
	{"         cs3   = acc0;                            ", 0x00,},
	{"         cs3   = acc1;                            ", 0x00,},
	{"         cs4   = acc0;                            ", 0x00,},
	{"         cs4   = acc1;                            ", 0x00,},
	{"         cs5   = acc0;                            ", 0x00,},
	{"         cs5   = acc1;                            ", 0x00,},
	{"                                                  ", 0x00,},
	{"                                                  ", 0x00,},
	{"                                                  ", 0x00,},
	{"                                                  ", 0x00,},

	{"         mute(0) = acc0;                          ", 0x00,},
	{"         mute(0) = acc1;                          ", 0x00,},
	{"         mute(1) = acc0;                          ", 0x00,},
	{"         mute(1) = acc1;                          ", 0x00,},
	{"         mute(2) = acc0;                          ", 0x00,},
	{"         mute(2) = acc1;                          ", 0x00,},
	{"         mute(3) = acc0;                          ", 0x00,},
	{"         mute(3) = acc1;                          ", 0x00,},
	{"         mute(4) = acc0;                          ", 0x00,},
	{"         mute(4) = acc1;                          ", 0x00,},
	{"         mute(5) = acc0;                          ", 0x00,},
	{"         mute(5) = acc1;                          ", 0x00,},
	{"                                                  ", 0x00,},
	{"                                                  ", 0x00,},
	{"                                                  ", 0x00,},
	{"                                                  ", 0x00,},

	{"jz    acc0   adrs(%3d);                           ", 0x20,},
	{"jn    acc0   adrs(%3d);                           ", 0x20,},
	{"jp    acc0   adrs(%3d);                           ", 0x20,},
	{"jump  acc0   adrs(%3d);                           ", 0x20,},
	{"         int0 = acc0;                             ", 0x00,},
	{"         int1 = acc0; // zero cross               ", 0x00,},
	{"acc0 +=  dbl                      <<%1d * lwr(0x%02x);", 0x05,},
	{"         ip = nov(acc0);                          ", 0x00,},
	{"                                                  ", 0x00,},
	{"                                                  ", 0x00,},
	{"         cb = acc0;                               ", 0x00,},
	{"         wb = acc0;                               ", 0x00,},
	{"acc0 += (iram(0xfc) = em)         <<%1d *     0x%02x ;", 0x05,},
	{"acc0 += (iram(0xfd) = em)         <<%1d *     0x%02x ;", 0x05,},
	{"acc0 += (iram(0xfe) = em)         <<%1d *     0x%02x ;", 0x05,},
	{"acc0 += (iram(0xff) = em)         <<%1d *     0x%02x ;", 0x05,},

	{"jz    acc0   adrs(%3d);                           ", 0x20,},
	{"jn    acc0   adrs(%3d);                           ", 0x20,},
	{"jp    acc0   adrs(%3d);                           ", 0x20,},
	{"jump  acc0   adrs(%3d);                           ", 0x20,},
	{"         int0 = acc0;                             ", 0x00,},
	{"         int1 = acc0; // zero cross               ", 0x00,},
	{"                                                  ", 0x00,},
	{"         ip = nov(acc0);                          ", 0x00,},
	{"                                                  ", 0x00,},
	{"                                                  ", 0x00,},
	{"         cb = acc0;                               ", 0x00,},
	{"         wb = acc0;                               ", 0x00,},
	{"acc0  = (iram(0xfc) = em)         <<%1d *     0x%02x ;", 0x05,},
	{"acc0  = (iram(0xfd) = em)         <<%1d *     0x%02x ;", 0x05,},
	{"acc0  = (iram(0xfe) = em)         <<%1d *     0x%02x ;", 0x05,},
	{"acc0  = (iram(0xff) = em)         <<%1d *     0x%02x ;", 0x05,},

	{"jz    acc1   adrs(%3d);                           ", 0x20,},
	{"jn    acc1   adrs(%3d);                           ", 0x20,},
	{"jp    acc1   adrs(%3d);                           ", 0x20,},
	{"jump  acc1   adrs(%3d);                           ", 0x20,},
	{"         int0 = acc1;                             ", 0x00,},
	{"         int1 = acc1; // zero cross               ", 0x00,},
	{"acc1 +=  dbl                      <<%1d * lwr(0x%02x);", 0x05,},
	{"         ip = nov(acc1);                          ", 0x00,},
	{"                                                  ", 0x00,},
	{"                                                  ", 0x00,},
	{"         cb = acc1;                               ", 0x00,},
	{"         wb = acc1;                               ", 0x00,},
	{"acc1 += (iram(0xfc) = em)         <<%1d *     0x%02x ;", 0x05,},
	{"acc1 += (iram(0xfd) = em)         <<%1d *     0x%02x ;", 0x05,},
	{"acc1 += (iram(0xfe) = em)         <<%1d *     0x%02x ;", 0x05,},
	{"acc1 += (iram(0xff) = em)         <<%1d *     0x%02x ;", 0x05,},

	{"jz    acc1   adrs(%3d);                           ", 0x20,},
	{"jn    acc1   adrs(%3d);                           ", 0x20,},
	{"jp    acc1   adrs(%3d);                           ", 0x20,},
	{"jump  acc1   adrs(%3d);                           ", 0x20,},
	{"         int0 = acc1;                             ", 0x00,},
	{"         int1 = acc1; // zero cross               ", 0x00,},
	{"                                                  ", 0x00,},
	{"         ip = nov(acc1);                          ", 0x00,},
	{"                                                  ", 0x00,},
	{"                                                  ", 0x00,},
	{"         cb = acc1;                               ", 0x00,},
	{"         wb = acc1;                               ", 0x00,},
	{"acc1  = (iram(0xfc) = em)         <<%1d *     0x%02x ;", 0x05,},
	{"acc1  = (iram(0xfd) = em)         <<%1d *     0x%02x ;", 0x05,},
	{"acc1  = (iram(0xfe) = em)         <<%1d *     0x%02x ;", 0x05,},
	{"acc1  = (iram(0xff) = em)         <<%1d *     0x%02x ;", 0x05,},
};

const sShowInst coef_func[] = {
	{"acc0  =  iram(0x%02x)               <<%1d * cs%1d      ;", 0x16,},
	{"acc0 +=  iram(0x%02x)               <<%1d * cs%1d      ;", 0x16,},
	{"acc1  =  iram(0x%02x)               <<%1d * cs%1d      ;", 0x16,},
	{"acc1 +=  iram(0x%02x)               <<%1d * cs%1d      ;", 0x16,},
	{"acc0  = (iram(0x%02x) = acc0)       <<%1d * cs%1d      ;", 0x16,},
	{"acc0 += (iram(0x%02x) = acc0)       <<%1d * cs%1d      ;", 0x16,},
	{"acc1  = (iram(0x%02x) = acc1)       <<%1d * cs%1d      ;", 0x16,},
	{"acc1 += (iram(0x%02x) = acc1)       <<%1d * cs%1d      ;", 0x16,},
	{"acc0  =  iram(0x%02x)               <<%1d * -cs%1d     ;", 0x16,},
	{"acc0 +=  iram(0x%02x)               <<%1d * -cs%1d     ;", 0x16,},
	{"acc1  =  iram(0x%02x)               <<%1d * -cs%1d     ;", 0x16,},
	{"acc1 +=  iram(0x%02x)               <<%1d * -cs%1d     ;", 0x16,},
	{"acc0  = (iram(0x%02x) = acc0)       <<%1d * -cs%1d     ;", 0x16,},
	{"acc0 += (iram(0x%02x) = acc0)       <<%1d * -cs%1d     ;", 0x16,},
	{"acc1  = (iram(0x%02x) = acc1)       <<%1d * -cs%1d     ;", 0x16,},
	{"acc1 += (iram(0x%02x) = acc1)       <<%1d * -cs%1d     ;", 0x16,},
	{"acc0  =  iram(0x%02x)               <<%1d * (1-cs%1d)  ;", 0x16,},
	{"acc0 +=  iram(0x%02x)               <<%1d * (1-cs%1d)  ;", 0x16,},
	{"acc1  =  iram(0x%02x)               <<%1d * (1-cs%1d)  ;", 0x16,},
	{"acc1 +=  iram(0x%02x)               <<%1d * (1-cs%1d)  ;", 0x16,},
	{"acc0  = (iram(0x%02x) = acc0)       <<%1d * (1-cs%1d)  ;", 0x16,},
	{"acc0 += (iram(0x%02x) = acc0)       <<%1d * (1-cs%1d)  ;", 0x16,},
	{"acc1  = (iram(0x%02x) = acc1)       <<%1d * (1-cs%1d)  ;", 0x16,},
	{"acc1 += (iram(0x%02x) = acc1)       <<%1d * (1-cs%1d)  ;", 0x16,},
	{"                                                  ", 0x00,}, 
	{"                                                  ", 0x00,}, 
	{"                                                  ", 0x00,}, 
	{"                                                  ", 0x00,}, 
	{"         iram(0x%02x) = sqr(acc0);                  ", 0x10,},
	{"         iram(0x%02x) = sqr(acc0);                  ", 0x10,},
	{"         iram(0x%02x) = sqr(acc1);                  ", 0x10,},
	{"         iram(0x%02x) = sqr(acc1);                  ", 0x10,},
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
/* PrintData3                                                   */
/*--------------------------------------------------------------*/
static void     PrintData3(int pram_offset, int pram_shift, int pram_coef, char* outInst, const sShowInst* theShowInst)
{        
	switch(theShowInst->coefType){
	case 0x00:
		sprintf(outInst, theShowInst->inst);
		break;
	case 0x05:
		sprintf(outInst, theShowInst->inst, shiftValue[pram_shift], pram_coef);
		break;
	case 0x10:
		sprintf(outInst, theShowInst->inst, pram_offset);
		break;
	case 0x15:
		sprintf(outInst, theShowInst->inst, pram_offset, shiftValue[pram_shift], pram_coef);
		break;
	case 0x16:
		sprintf(outInst, theShowInst->inst, pram_offset, shiftValue[pram_shift], (pram_coef & 0xe0) >> 5);
		break;
	case 0x19:
		sprintf(outInst, theShowInst->inst, ((pram_shift & 0x02) >> 1), 
			pram_offset, shiftValue[(pram_shift & 0x01)], pram_coef);
		break;
	case 0x20:
		sprintf(outInst, theShowInst->inst, ((pram_shift << 8) + pram_coef));
	default:
		break;
	}
}
/*--------------------------------------------------------------*/
/* PrintData2                                                   */
/*--------------------------------------------------------------*/
static void     PrintData2(void)
{        
	int	i,j;
	char   	currInst[256];
	int	eram_offset;
	int	instType;
	int	eram_count = 0;

	int	pram_inst;
	int	pram_offset;
	int	pram_shift;
	int	pram_coef;

	for (j = 0; j < 2; j++){
		printf("processor %d\n", j);
		for (i = 0; i < ERAMSTEP; i++){

			pram_inst   = (prog[i][j] & 0x007c0000) >> 18;
			pram_offset = (prog[i][j] & 0x0003fc00) >> 10;
			pram_shift  = (prog[i][j] & 0x00000300) >> 8;
			pram_coef   = (prog[i][j] & 0x000000ff);
			switch (pram_inst){
			case 0x0d: // decode
				if(pram_offset >= 0xa0) {
					PrintData3(pram_offset, pram_shift, pram_coef, currInst,
						   &decode[pram_offset - 0xa0]);
				}
				break;
			case 0x0c:
				PrintData3(pram_offset, pram_shift, pram_coef, currInst,
					   &coef_func[pram_coef & 0x1f]);
				break;
			case 0x00:
				if (prog[i][j] & 0x0003ffff) {
					PrintData3(pram_offset, pram_shift, pram_coef, currInst,
						   &inst[pram_inst]);
				} else {
					sprintf(currInst, "                                                  ");
				}
				break;
			default:
				PrintData3(pram_offset, pram_shift, pram_coef, currInst,
					   &inst[pram_inst]);
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

			printf(" %2d%4d 0x%08x  %s",
				       j, i, prog[i][j], currInst);

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
