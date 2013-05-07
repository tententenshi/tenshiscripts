typedef union _UCompPrm 
{
        short uPrm[17];
        struct {
                short lowSplit;
                short highSplit;
                struct {
                        short threshold;
                        short ratio;
                        short attack;
                        short release;
                        short level;
                } low;
                struct {
                        short threshold;
                        short ratio;
                        short attack;
                        short release;
                        short level;
                } mid;
                struct {
                        short threshold;
                        short ratio;
                        short attack;
                        short release;
                        short level;
                } high;
        } sPrm;
} UCompPrm;

int main(void)
{
	static UCompPrm tempCompPrm, currCompPrm;
	tempCompPrm.uPrm[0] = 0x01;
	tempCompPrm.uPrm[1] = 0x23;
	tempCompPrm.uPrm[2] = 0x34;
	tempCompPrm.uPrm[3] = 0x45;
	tempCompPrm.uPrm[4] = 0x56;
	tempCompPrm.uPrm[5] = 0x67;
	tempCompPrm.uPrm[6] = 0x78;
	tempCompPrm.uPrm[7] = 0x89;
	tempCompPrm.uPrm[8] = 0x9a;
	tempCompPrm.uPrm[9] = 0xab;
	tempCompPrm.uPrm[10] = 0xbc;
	tempCompPrm.uPrm[11] = 0xcd;
	tempCompPrm.uPrm[12] = 0xde;
	tempCompPrm.uPrm[13] = 0xef;
	tempCompPrm.uPrm[14] = 0xf0;
	tempCompPrm.uPrm[15] = 0x01;
	tempCompPrm.uPrm[16] = 0x12;
	currCompPrm.uPrm[0] = 0xff;
	currCompPrm.uPrm[1] = 0xff;
	currCompPrm.uPrm[2] = 0xff;
	currCompPrm.uPrm[3] = 0xff;
	currCompPrm.uPrm[4] = 0xff;
	currCompPrm.uPrm[5] = 0xff;
	currCompPrm.uPrm[6] = 0xff;
	currCompPrm.uPrm[7] = 0xff;
	currCompPrm.uPrm[8] = 0xff;
	currCompPrm.uPrm[9] = 0xff;
	currCompPrm.uPrm[10] = 0xff;
	currCompPrm.uPrm[11] = 0xff;
	currCompPrm.uPrm[12] = 0xff;
	currCompPrm.uPrm[13] = 0xff;
	currCompPrm.uPrm[14] = 0xff;
	currCompPrm.uPrm[15] = 0xff;
	currCompPrm.uPrm[16] = 0xff;
	tempCompPrm = currCompPrm;
	
	printf("prm 0: lowSplit     = 0x%02x\n", tempCompPrm.sPrm.lowSplit);
	printf("prm 1: highSplit    = 0x%02x\n", tempCompPrm.sPrm.highSplit);
	printf("prm 2: low-thres    = 0x%02x\n", tempCompPrm.sPrm.low.threshold);
	printf("prm 3: low-ratio    = 0x%02x\n", tempCompPrm.sPrm.low.ratio);
	printf("prm 4: low-attack   = 0x%02x\n", tempCompPrm.sPrm.low.attack);
	printf("prm 5: low-release  = 0x%02x\n", tempCompPrm.sPrm.low.release);
	printf("prm 6: low-level    = 0x%02x\n", tempCompPrm.sPrm.low.level);
	printf("prm 7: mid-thres    = 0x%02x\n", tempCompPrm.sPrm.mid.threshold);
	printf("prm 8: mid-ratio    = 0x%02x\n", tempCompPrm.sPrm.mid.ratio);
	printf("prm 9: mid-attack   = 0x%02x\n", tempCompPrm.sPrm.mid.attack);
	printf("prm10: mid-release  = 0x%02x\n", tempCompPrm.sPrm.mid.release);
	printf("prm11: mid-level    = 0x%02x\n", tempCompPrm.sPrm.mid.level);
	printf("prm12: high-thres   = 0x%02x\n", tempCompPrm.sPrm.high.threshold);
	printf("prm13: high-ratio   = 0x%02x\n", tempCompPrm.sPrm.high.ratio);
	printf("prm14: high-attack  = 0x%02x\n", tempCompPrm.sPrm.high.attack);
	printf("prm15: high-release = 0x%02x\n", tempCompPrm.sPrm.high.release);
	printf("prm16: high-level   = 0x%02x\n", tempCompPrm.sPrm.high.level);

}
