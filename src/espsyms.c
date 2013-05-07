/*
 *==============================================================
 *
 *      espsyms.c
 *
 *              $Author: tnisimu $
 *              $Revision: 1.3 $
 *              $Date: 2003/07/10 08:11:37 $
 *
 *==============================================================
 */
#include        <stdio.h>
#include        <stdlib.h>

#include        <string.h>


#define MAXSTEP		2048
#define MAXBUF          2048
#define MAXDATA         1024
#define	PROCESSOR1_START_ADS	1024

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
        InfoNG = -1,
        InfoSglCoef = 0,
        InfoDblCoef,
        InfoIram,
        InfoEramRd,
        InfoEramWr,
        InfoEramOther
};

typedef struct _SData {
        char            name[MAXBUF];
        unsigned char   shift;
        unsigned long   value;
        char            type[MAXDATA];
        short           step[MAXDATA];
} SData;

unsigned long   prog[MAXSTEP];
SData           cdat[MAXDATA];
SData           idat[MAXDATA];
SData           edat[MAXDATA];


/*--------------------------------------------------------------*/
/* InitSData                                                    */
/*--------------------------------------------------------------*/
static void     InitSData(SData *sdat)
{
        int     i, j;
        
        for (i = 0; i < MAXDATA; i++) {
                (sdat + i)->name[0] = '\0';
                (sdat + i)->shift = 0;
                (sdat + i)->value = 0;
                for (j = 0; j < MAXDATA; j++) {
                        (sdat + i)->type[j] = InfoNG;
                        (sdat + i)->step[j] = -1;
                }
        }
}

/*--------------------------------------------------------------*/
/* InitData                                                     */
/*--------------------------------------------------------------*/
static void     InitData(void)
{
        int     i;
        
        for (i = 0; i < MAXSTEP; i++) {
                prog[i] = 0;
        }
        InitSData(cdat);
        InitSData(idat);
        InitSData(edat);
}

/*--------------------------------------------------------------*/
/* PrintSData                                                   */
/*--------------------------------------------------------------*/
static void     PrintSData(FILE *fp, SData* sdat, char *prefix, char *sym)
{
        int     i, j;
        
        for (i = 0; i < MAXDATA; i++, sdat++) {
                for (j = 0; j < MAXDATA; j++) {
                        if (sdat->type[j] < 0) {
                                break;
                        }
                        if (sdat->step[j] < 0) {
                                break;
                        }
                        fprintf(fp, "#define %s%s%s_%d\t0x%02X%02X%04X\n",
				prefix, sym, sdat->name, j, sdat->type[j], sdat->shift, sdat->step[j]);
                }
        }
}

/*--------------------------------------------------------------*/
/* PrintSDataIOfst						*/
/*--------------------------------------------------------------*/
static void     PrintSDataIOfst(FILE *fp, SData* sdat, char *prefix, char *sym)
{
        int     i;
        
        for (i = 0; i < MAXDATA; i++, sdat++) {
		if (sdat->type[0] < 0) {
			break;
		}
		if (sdat->step[0] < 0) {
			break;
		}
		fprintf(fp, "#define %s%s%s\t0x%02X\n",
			prefix, sym, sdat->name, sdat->value);
        }
}

/*--------------------------------------------------------------*/
/* PrintSDataEOfst						*/
/*--------------------------------------------------------------*/
static void     PrintSDataEOfst(FILE *fp, SData* sdat, char *prefix, char *sym)
{
        int     i;
        
        for (i = 0; i < MAXDATA; i++, sdat++) {
		if (sdat->type[0] < 0) {
			break;
		}
		if (sdat->step[0] < 0) {
			break;
		}
		fprintf(fp, "#define %s%s%s\t0x%06X\n",
			prefix, sym, sdat->name, sdat->value);
	}
}

/*--------------------------------------------------------------*/
/* PrintData2                                                   */
/*--------------------------------------------------------------*/
static void     PrintData2(FILE* fp, char *prefix)
{        
        /*----- Print Coef Data -----*/
        fprintf(fp, "/*===== COEF =====*/\n");
        PrintSData(fp, cdat, prefix, "COE_");
        fprintf(fp, "\n");

        /*----- Print Iram Data -----*/
        fprintf(fp, "/*===== IRAM =====*/\n");
        PrintSData(fp, idat, prefix, "IRM_");
        fprintf(fp, "\n");

        /*----- Print Iram Ofst -----*/
        fprintf(fp, "/*===== IRAM Ofst =====*/\n");
        PrintSDataIOfst(fp, idat, prefix, "IOF_");
        fprintf(fp, "\n");

        /*----- Print Eram Data -----*/
        fprintf(fp, "/*===== ERAM =====*/\n");
        PrintSData(fp, edat, prefix, "ERM_");
        fprintf(fp, "\n");

        /*----- Print Eram Ofst -----*/
        fprintf(fp, "/*===== ERAM Ofst =====*/\n");
        PrintSDataEOfst(fp, edat, prefix, "EOF_");
        fprintf(fp, "\n");

}

/*--------------------------------------------------------------*/
/* Usage                                                        */
/*--------------------------------------------------------------*/
static void     Usage(int out)
{
        fprintf(stderr,"usage:\n");
        fprintf(stderr,"  mr3syms <obj file name> <output file name.h> \"prefix\"\n");
        exit(out);
}
/*--------------------------------------------------------------*/
/* Error                                                        */
/*--------------------------------------------------------------*/
static void     Error(char *s, FILE* fp1, FILE* fp2)
{
        fprintf(stderr,"%s",s);
        fclose(fp1);
        fclose(fp2);
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
	char		processorNum;
        unsigned long   progDat;
        
	processorNum = buf[0];

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

        if (processorNum == '1'){
		step += PROCESSOR1_START_ADS;
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
        prog[step] = progDat;
        
        return(0);
}

/*--------------------------------------------------------------*/
/* GetCoefData                                                  */
/*--------------------------------------------------------------*/
enum {
        GET_COEF_NAME = 0,
        COEF_NOP1,
        GET_COEF_TYPE,
        COEF_NOP2,
        GET_COEF_VALUE,
        COEF_NOP3,
	GET_COEF_PROCESSOR,
        COEF_NOP4,
        GET_COEF_STEP
};
static int      GetCoefData(char* buf, SData* cDat)
{
        int     name_i = 0;
        int     step_i = 0;
        int     step_num = 0;
        int     status;
        char    tmp[10];
	char	processorNum;

        cDat->shift = 0;        /*<--- Not Support Now!! */
        cDat->value = 0;        /*<--- Not Support Now!! */
        
        if ((*buf == ' ') || (*buf == '\0') || (*buf == '\n')) {
                return(1);
        } else {
                status = GET_COEF_NAME;
        }
        
        for (;*buf != '\0'; buf++) {
                switch (status) {
                case GET_COEF_NAME:
                        if (*buf != ' ') {
                                cDat->name[name_i++] = *buf;
                        } else {
                                cDat->name[name_i] = '\0';
                                status = COEF_NOP1;
                        }
                        break;
                case GET_COEF_TYPE:
                        switch (*buf) {
                         case 's':      cDat->type[0] = InfoSglCoef;    break;
                         case 'h':      cDat->type[0] = InfoDblCoef;    break;
                         case 'l':      cDat->type[0] = InfoNG;         break;
                         default:       return(1);                      break;
                        }
                        status = COEF_NOP2;
                        break;
                case GET_COEF_VALUE:
                        if (*buf == ' ') {
                                status = COEF_NOP3;
                        }
                        break;
                case GET_COEF_PROCESSOR:
                        if (*buf == ';') {
                                return(0);
                        } else if (*buf == ' ') {
                                status = COEF_NOP4;
                        } else {
				processorNum = *buf;
			}

                        break;
                case GET_COEF_STEP:
                        if (*buf == ' ') {
                                tmp[step_i++] = '\0';
                                cDat->type[step_num] = cDat->type[0];
                                cDat->step[step_num] = (short)strtol(tmp, NULL, 10);
				if (processorNum == '1'){
					cDat->step[step_num] += PROCESSOR1_START_ADS;
				}
				step_num++;
                                status = COEF_NOP3;
                                step_i = 0;
                        } else {
                                tmp[step_i++] = *buf;
                        }
                        break;
                default:
                        if ((*buf == ' ') || (*buf == '\0') || (*buf == '\n')) {
                                /* Skip */
                        } else {
                                buf--;
                                status++;
                        }
                        break;
                }
        }
        
        return(1);
}

/*--------------------------------------------------------------*/
/* GetIramData                                                  */
/*--------------------------------------------------------------*/
enum {
        GET_IRAM_NAME = 0,
        IRAM_NOP1,
        GET_IRAM_VALUE,
        IRAM_NOP2,
	GET_IRAM_PROCESSOR,
        IRAM_NOP3,
        GET_IRAM_STEP
};
static int      GetIramData(char* buf, SData* iDat)
{
        int     name_i = 0;
        int     val_i  = 0;
        int     step_i = 0;
        int     step_num = 0;
        int     status;
        char    tmp[10];
	char	processorNum;
        
        iDat->shift = 0;        /*<--- Not Support Now!! */
//		iDat->value = 0;        /*<--- Not Support Now!! */
                
        if ((*buf == ' ') || (*buf == '\0') || (*buf == '\n')) {
                return(1);
        } else {
                status = GET_IRAM_NAME;
        }
        
        for (;*buf != '\0'; buf++) {
                switch (status) {
                case GET_IRAM_NAME:
                        if (*buf != ' ') {
                                iDat->name[name_i++] = *buf;
                        } else {
                                iDat->name[name_i] = '\0';
                                status = IRAM_NOP1;
                        }
                        break;
                case GET_IRAM_VALUE:
                        if (*buf == ' ') {
                                tmp[val_i++] = '\0';
                                iDat->value = (short)strtol(tmp, NULL, 16);
                                status = IRAM_NOP2;
                        }else{
                                tmp[val_i++] = *buf;
			}
                        break;
                case GET_IRAM_PROCESSOR:
                        if (*buf == ';') {
                                return(0);
                        } else if (*buf == ' ') {
                                status = IRAM_NOP3;
                        } else {
                                processorNum = *buf;
                        }
                         
                        break;
                case GET_IRAM_STEP:
                        if (*buf == ' ') {
                                tmp[step_i++] = '\0';
                                iDat->type[step_num] = InfoIram;
                                iDat->step[step_num] = (short)strtol(tmp, NULL, 10);
                                if (processorNum == '1'){
                                        iDat->step[step_num] += PROCESSOR1_START_ADS;
                                }
                                step_num++;
                                status = IRAM_NOP2;
                                step_i = 0;
                        } else {
                                tmp[step_i++] = *buf;
                        }
                        break;
                default:
                        if ((*buf == ' ') || (*buf == '\0') || (*buf == '\n')) {
                                /* Skip */
                        } else {
                                buf--;
                                status++;
                        }
                        break;
                }
        }
        
        return(1);
}


/*--------------------------------------------------------------*/
/* GetEramData                                                  */
/*--------------------------------------------------------------*/
enum {
        GET_ERAM_NAME = 0,
        ERAM_NOP1,
        GET_ERAM_VALUE,
        ERAM_NOP2,
        GET_ERAM_STEP
};

static int      GetEramData(char* buf, SData* eDat)
{
        int             name_i = 0;
	int		val_i = 0;
        int             step_i = 0;
        int             step_num = 0;
        int             status;
        char            tmp[10];
        unsigned long   instTmp;
        
        eDat->shift = 0;        /*<--- Not Support Now!! */
        eDat->value = 0;        /*<--- Not Support Now!! */
                
        if ((*buf == ' ') || (*buf == '\0') || (*buf == '\n')) {
                return(1);
        } else {
                status = GET_ERAM_NAME;
        }
        
        for (;*buf != '\0'; buf++) {
                switch (status) {
                case GET_ERAM_NAME:
                        if (*buf != ' ') {
                                eDat->name[name_i++] = *buf;
                        } else {
                                eDat->name[name_i] = '\0';
                                status = ERAM_NOP1;
                        }
                        break;
                case GET_ERAM_VALUE:
                        if (*buf == ' ') {
                                tmp[val_i++] = '\0';
                                eDat->value = strtol(tmp, NULL, 16);
                                status = ERAM_NOP2;
                        }else{
                                tmp[val_i++] = *buf;
			}
                        break;
                case GET_ERAM_STEP:
                        if (*buf == ';') {
                                return(0);
                        } else if (*buf == ' ') {
                                tmp[step_i++] = '\0';
                                instTmp = eDat->step[step_num] = (short)strtol(tmp, NULL, 10) + PROCESSOR1_START_ADS;
                                instTmp = (prog[instTmp] >> 26) & 0x3;
                                switch (instTmp) {
				case 0x1:
                                        eDat->type[step_num] = InfoEramRd;
                                        break;
				case 0x2:
                                        eDat->type[step_num] = InfoEramWr;
                                        break;
				default:
					eDat->type[step_num] = InfoEramOther;
                                        break;
                                }
                                step_num++;
                                status = ERAM_NOP2;
                                step_i = 0;
                        } else {
                                tmp[step_i++] = *buf;
                        }
                        break;
                default:
                        if ((*buf == ' ') || (*buf == '\0') || (*buf == '\n')) {
                                /* Skip */
                        } else {
                                buf--;
                                status++;
                        }
                        break;
                }
        }
        
        return(1);
}


/*--------------------------------------------------------------*/
/* CheckData                                                    */
/*--------------------------------------------------------------*/
static unsigned char      CheckData(FILE *fp)
{
        char            lbuf[MAXBUF], lbuf2[MAXBUF];
        int             status = NOP;
        unsigned char   flag = 0;
        SData*          cDat = cdat;
        SData*          iDat = idat;
        SData*          eDat = edat;
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
                                flag = (1 << COEF);
                                if (GetCoefData(lbuf, cDat++)) {
                                        return(1);
                                }
                                break;
                        case IRAM:
                                flag |= (1 << IRAM);
                                if (GetIramData(lbuf, iDat++)) {
                                        return(1);
                                }
                                break;
                        case ERAM:
                                flag = (1 << ERAM);
                                if (GetEramData(lbuf, eDat++)) {
                                        return(1);
                                }
                                break;
                        case LABL:
                                flag = (1 << LABL);
                                break;
                        case GRAM:
                                flag = (1 << GRAM);
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
	char	*prefix = "\0";
        FILE    *inpFps;
        FILE    *outFps2;

//		int     i;
        
        /*----- init. data -----*/
        InitData();
        
        /*----- option check -----*/
        fprintf(stderr, "\n");
        if (argc < 3 || argc > 4) {
                Usage(1);
        }
	if (argc == 4){
		prefix = argv[3];
	}
        fprintf(stderr, "--- MR3 data convert tool ---\n");
        fprintf(stderr, "               Version 0.5\n");
        fprintf(stderr, "\n");
        
        /*----- Open Files -----*/
        inpFps = fopen(argv[1], "r");
        outFps2 = fopen(argv[2], "wr");
        if (inpFps == NULL) {
                Error("Can not find input file!\n", inpFps, outFps2);
        }

        if (outFps2 == NULL) {
                Error("Can not open output file2!\n", inpFps, outFps2);
        }
        
        /*----- check data -----*/
        if (flag = CheckData(inpFps)) {
		if (flag & 0x01) fprintf(stderr, "OBJS Error\n");
		else if (flag & 0x02) fprintf(stderr, "COEF Error\n");
		else if (flag & 0x08) fprintf(stderr, "ERAM Error\n");
		else  fprintf(stderr, "Other Error\n");
                Error("Invalid input file!\n", inpFps, outFps2);
        }
        
        /*----- print data -----*/
        PrintData2(outFps2, prefix);

        fclose(inpFps);
        fclose(outFps2);
        exit(0);
}
