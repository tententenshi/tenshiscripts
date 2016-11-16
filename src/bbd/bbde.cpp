//
// BBD device emulator  ( not delay emulator )
//
#include <stdio.h>
#include <math.h>

double	buf_441[16], buf_bbd[512];		// delay buf
int		phai_441, phai_bbd;				// integer part of phase
double	phaf_bbd, tmp_441, tmp_bbd;		// float phase
double	inc_bbd;						// bbd phase inc ( 1.0: 44.1k system sample rate )
double	inv_inc;						// inverse of inc_bbd
double	mxi, mxo;						// i/o data
int		dat_flg;						// input data flag for debug
int		bp_441, bp_bbd;					// base ptr ( dec by sample(441) / dec by write(bbd) )

void	init(void);
double	proc(void);
void	write_bbd(double, double);
double	read_bbd(void);
void	write_441(void);
double	lerp(double, double, double);

int main(int argc, char *argv[])
{
	int		i;

	init();
	inc_bbd = 0.2;													// bbd clock initial value
	for (i=0; i<0x40; i++) {										// sample period
		if (i == 0x2) {												// input data: width=4, timing=0x2
			dat_flg = 4;
		}
		else if (dat_flg) {
			--dat_flg;
		}
		mxo = proc();												// bbd emu
//		inc_bbd -= 0.11;											// bbd clock modulation
	}
	return 0;
}
// --------------------------------------------------------------------
double proc(void)
{
	double	rd;

	inv_inc = 1.0 / inc_bbd;
	rd = read_bbd();												// SRC bbd -> 441
	phai_441 = (int)(phaf_bbd + inc_bbd);							// SRC 441 -> bbd
	if (phai_441 != 0) {											// if carry -> normalize bbd phase to next integer position
		tmp_441 = (1.0 - phaf_bbd) * inv_inc;
		tmp_bbd = ((int)phaf_bbd) + 1.0;
		write_bbd(tmp_bbd, tmp_441);
		bp_bbd = --bp_bbd & 0x1ff;
//
		while ( --phai_441 > 0) {									// if more carry -> next integer position
			tmp_441 = tmp_441 + inv_inc;
			tmp_bbd = tmp_bbd + 1.0;
			write_bbd(tmp_bbd, tmp_441);
			bp_bbd = --bp_bbd & 0x1ff;
		}
	}
	phai_bbd = (int)(phaf_bbd + inc_bbd);							// post proc / update for next sample period
	phaf_bbd = phaf_bbd + inc_bbd - phai_bbd;
//
	write_441();
	bp_441 = --bp_441 & 0xf;
	return rd;
}
// --------------------------------------------------------------------
void init(void)
{
	int	i;

	for (i=0; i<0x10; i++) {
		buf_441[i] = 0.0;
	}
	for (i=0; i<0x200; i++) {
		buf_bbd[i] = 0.0;
	}
	for (i=0x1f4; i<0x1f8; i++) {									// initial contents for SRC bbd -> 441
		buf_bbd[i] = 1.0;
	}
	bp_441 = 0;
	bp_bbd = 0;
	phaf_bbd = 0.0;
	dat_flg = 0;
	mxi = 1.0;
}
// --------------------------------------------------------------------
void write_bbd(double phab, double pha4)
{
	int		bp;
	double	wd;

	bp = bp_441 + 2;												// 2: offset for interpolation
	wd = lerp(buf_441[(bp - 1) & 0xf], buf_441[bp & 0xf], pha4);
	buf_bbd[bp_bbd] = wd;
//	printf("     write_bbd  inc: %9.6lf  bp_441: %01x  phab: %9.6lf  bp_bbd: %03x  buf_bbd[%03x]=%9.6lf\n",
//								 inc_bbd,        bp_441,     phab,           bp_bbd,       bp_bbd, wd);
}
// --------------------------------------------------------------------
double lerp(double d0, double d1, double k)
{
	return d0 * (1.0 - k) + d1 * k;
}
// --------------------------------------------------------------------
double read_bbd(void)
{
	int		bp;
	double	rd;

	bp = bp_bbd + 0x1fc;											// 0x1fc: bbd cell count(512) - some latency@input
	rd = lerp(buf_bbd[(bp - 1) & 0x1ff], buf_bbd[bp & 0x1ff], (1.0 - phaf_bbd));
//	aaf((inc_bbd > 1.0) ? inv_inc : 1.0);							// anti ariae filter
	printf("     read_bbd   inc: %9.6lf  bp_441: %01x  phab: %9.6lf  bp_bbd: %03x  mxo=%9.6lf\n",
								 inc_bbd,        bp_441,     phaf_bbd,       bp_bbd,   rd);
	return rd;
}
// --------------------------------------------------------------------
void write_441(void)
{
//	aaf((inc_bbd < 1.0) ? inc_bbd : 1.0);							// anti arias flter
	buf_441[bp_441] = dat_flg ? mxi : 0.0;
//	printf("---- write_441  inc: %9.6lf  bp_441: %01x  phab: %9.6lf  bp_bbd: %03x  buf_441\[%01x]=%9.6lf\n",
//								 inc_bbd,        bp_441,     phaf_bbd,       bp_bbd,        bp_441, dat_flg ? mxi : 0.0);
}
// --------------------------------------------------------------------
