#include <stdio.h>

int g;

int *subsub()
{
	int *p;

	p = &g;
	p = NULL;

	return p;
}

int sub()
{
	int a;
	int *ptr;

	ptr = subsub();
	a = *ptr;

	return a * 10;
}

main()
{
	g = 3;
	printf ("%d\n", sub());

	return 0;
}

