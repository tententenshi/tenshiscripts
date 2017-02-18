#include <stdio.h>
#include <GL/glut.h>

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
#if 0
	glColor3d(1.0, 0.0, 0.0);
//	glBegin(GL_LINE_LOOP);
	glBegin(GL_POLYGON);
	glColor3d(1.0, 0.0, 0.0);	/* 赤 */
	glVertex2d(-0.9, -0.9);
	glColor3d(0.0, 1.0, 0.0);	/* 緑 */
	glVertex2d(0.9, -0.9);
	glColor3d(0.0, 0.0, 1.0);	/* 青 */
	glVertex2d(0.9, 0.9);
	glColor3d(1.0, 1.0, 0.0);	/* 黄 */
	glVertex2d(-0.9, 0.9);
	glEnd();
#endif
	glFlush();
}

void resize(int w, int h)
{
	/* ウィンドウ全体をビューポートにする */
	glViewport(0, 0, w, h);

	/* 変換行列の初期化 */
	glLoadIdentity();

#if 0
	/* スクリーン上の表示領域をビューポートの大きさに比例させる */
	glOrtho(-w / 200.0, w / 200.0, -h / 200.0, h / 200.0, -1.0, 1.0);
#else
	/* スクリーン上の座標系をマウスの座標系に一致させる */
	glOrtho(-0.5, (GLdouble)w - 0.5, (GLdouble)h - 0.5, -0.5, -1.0, 1.0);
#endif
}

void mouse(int button, int state, int x, int y)
{
	static int x0, y0;
	
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_UP) {
			/* ボタンを押した位置から離した位置まで線を引く */
			glColor3d(0.0, 0.0, 0.0);
			glBegin(GL_LINES);
			glVertex2i(x0, y0);
			glVertex2i(x, y);
			glEnd();
			glFlush();
		} else {
			/* ボタンを押した位置を覚える */
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
