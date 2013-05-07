#include <stdio.h>
#include <math.h>

int     ma;		// 24 bit
int     mb;		// 16 bit
int     mp;		// 28 bit
int     s;		// 2 bit    0: thru, 1: <<1, 2: <<2, 3: <<4
int     aa, ab, ac;	// 28 bit

int mul_func(int, int, int);
int add_func(int, int);
int rtz(int);

int main()
{
	int i;
	int din;
	int dly;

	din = -0x000423; //-0x004c00;
	dly = din;
	din = 0;
	
	for (i=0; i<5000; i++) {
		ma = dly;
		mb = 0x7ffc;
		s = 0;
		mp = mul_func(ma, mb, s);
		aa = din << 4;
		ab = mp;
		ac = add_func(aa, ab);
//		dly = ac >> 4;		// without rtz
		dly = rtz(ac) >> 4;
		printf("0x%06x * 0x%04x = 0x%07x, 0x%07x + 0x%07x = 0x%07x, iram = 0x%06x\n",
		       ma & 0xffffff, mb & 0xffff, mp & 0xfffffff,
		       aa & 0xfffffff, ab & 0xfffffff, ac & 0xfffffff,
		       dly & 0xffffff);

	}
#if 0
	ma = 0x001234;
	mb = -0x2;
	s = 0;
	ac = 0;

	// mul
	mp = mul_func(ma, mb, s);
	printf("%06x * %04x = %07x\n", ma & 0xffffff, mb & 0xffff, mp & 0xfffffff);

	// add
	aa = mp >> 4;
	ab = ac;
	ac = add_func(aa, ab);
	printf("%07x + %07x = %07x\n", aa & 0xfffffff, ab & 0xfffffff, ac & 0xfffffff);
#endif
}

int mul_func(int a, int b, int s)
{
	long long tmp;

	tmp = (long long)a * (long long)b;
	switch (s) {
	case 0: tmp >>= 11;
		break;
	case 1: tmp >>= 10;
		break;
	case 2: tmp >>= 9;
		break;
	case 3: tmp >>= 7;
		break;
	}
	
	if (tmp < -0x8000000LL)
		return -0x8000000;
	else if (tmp >= 0x8000000)
		return 0x7ffffff;
	else
		return (int)tmp;
}

int add_func(int a, int b)
{
	int tmp;

	tmp = a + b;
	if (tmp < -0x8000000)
		return -0x8000000;
	else if (tmp >= 0x8000000)
		return 0x7ffffff;
	else
		return tmp;
}

int rtz(int a)
{
	int tmp;

	tmp = a & -0x0000010;
	if (tmp < 0)
		return tmp + 0x0000010;
	else
		return tmp;
}
