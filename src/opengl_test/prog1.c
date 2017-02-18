#include <stdio.h>
#include <GL/glut.h>

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
#if 0
	glColor3d(1.0, 0.0, 0.0);
//	glBegin(GL_LINE_LOOP);
	glBegin(GL_POLYGON);
	glColor3d(1.0, 0.0, 0.0);	/* $B@V(B */
	glVertex2d(-0.9, -0.9);
	glColor3d(0.0, 1.0, 0.0);	/* $BNP(B */
	glVertex2d(0.9, -0.9);
	glColor3d(0.0, 0.0, 1.0);	/* $B@D(B */
	glVertex2d(0.9, 0.9);
	glColor3d(1.0, 1.0, 0.0);	/* $B2+(B */
	glVertex2d(-0.9, 0.9);
	glEnd();
#endif
	glFlush();
}

void resize(int w, int h)
{
	/* $B%&%#%s%I%&A4BN$r%S%e!<%]!<%H$K$9$k(B */
	glViewport(0, 0, w, h);

	/* $BJQ499TNs$N=i4|2=(B */
	glLoadIdentity();

#if 0
	/* $B%9%/%j!<%s>e$NI=<(NN0h$r%S%e!<%]!<%H$NBg$-$5$KHfNc$5$;$k(B */
	glOrtho(-w / 200.0, w / 200.0, -h / 200.0, h / 200.0, -1.0, 1.0);
#else
	/* $B%9%/%j!<%s>e$N:BI87O$r%^%&%9$N:BI87O$K0lCW$5$;$k(B */
	glOrtho(-0.5, (GLdouble)w - 0.5, (GLdouble)h - 0.5, -0.5, -1.0, 1.0);
#endif
}

void mouse(int button, int state, int x, int y)
{
	static int x0, y0;
	
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_UP) {
			/* $B%\%?%s$r2!$7$?0LCV$+$iN%$7$?0LCV$^$G@~$r0z$/(B */
			glColor3d(0.0, 0.0, 0.0);
			glBegin(GL_LINES);
			glVertex2i(x0, y0);
			glVertex2i(x, y);
			glEnd();
			glFlush();
		} else {
			/* $B%\%?%s$r2!$7$?0LCV$r3P$($k(B */
			x0 = x;
			y0 = y;
		}
		printf("left");
		break;
	case GLUT_MIDDLE_BUTTON:
		printf("middle");
		break;
	case GLUT_RIGHT_BUTTON:
		printf("right");
		break;
	default:
		break;
	}

	printf(" button is ");

	switch (state) {
	case GLUT_UP:
		printf("up");
		break;
	case GLUT_DOWN:
		printf("down");
		break;
	default:
		break;
	}

	printf(" at (%d, %d)\n", x, y);
}

void init(void)
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

int main(int argc, char *argv[])
{
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(320, 240);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutCreateWindow(argv[0]);
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutMouseFunc(mouse);
	init();
	glutMainLoop();
	return 0;
}
