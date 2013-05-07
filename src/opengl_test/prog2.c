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

void init(void)
{
//	glClearColor(0.0, 0.0, 1.0, 1.0);
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutCreateWindow(argv[0]);
	glutDisplayFunc(display);
	init();
	glutMainLoop();
	return 0;
}

