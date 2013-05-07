#include <GL/glut.h>

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3d(1.0, 0.0, 0.0);
//	glBegin(GL_LINE_LOOP);
	glBegin(GL_POLYGON);
	glColor3d(1.0, 0.0, 0.0); /* $B@V(B */
	glVertex2d(-0.9, -0.9);
	glColor3d(0.0, 1.0, 0.0); /* $BNP(B */
	glVertex2d(0.9, -0.9);
	glColor3d(0.0, 0.0, 1.0); /* $B@D(B */
	glVertex2d(0.9, 0.9);
	glColor3d(1.0, 1.0, 0.0); /* $B2+(B */
	glVertex2d(-0.9, 0.9);
	glEnd();
	glFlush();
}

void resize(int w, int h)
{
	/* $B%&%#%s%I%&A4BN$r%S%e!<%]!<%H$K$9$k(B */
	glViewport(0, 0, w, h);

	/* $BJQ499TNs$N=i4|2=(B */
	glLoadIdentity();

	/* $B%9%/%j!<%s>e$NI=<(NN0h$r%S%e!<%]!<%H$NBg$-$5$KHfNc$5$;$k(B */
	glOrtho(-w / 200.0, w / 200.0, -h / 200.0, h / 200.0, -1.0, 1.0);
}


void init(void)
{
//	glClearColor(0.0, 0.0, 1.0, 1.0);
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

int main(int argc, char *argv[])
{
	glutInitWindowPosition(400, 400);
	glutInitWindowSize(320, 240);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutCreateWindow(argv[0]);
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	init();
	glutMainLoop();
	return 0;
}

