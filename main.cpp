#include "final.h"
#include <stdio.h>

extern int current_width, current_height;

extern vector<Vec2f> g_pPosition;
extern int g_nX, g_nY;
extern int g_nSelect;
extern int g_nGLWidth, g_nGLHeight;


extern float PI;

extern float theta, phi, radius;
extern float stheta, sphi, sradius;
extern Vec3f cam, scam;

int lightMode = 0;
int mouseBut = 0;
extern int mode;
bool halfMode = false;
extern bool done;
extern int step;
extern bool lock;
extern bool assemble;
extern bool keyMode;
extern bool mPick;
extern int itemPick;
extern int lpSelect;
extern bool hit;
extern bool pinlock[5];
extern bool lpPick, k1Pick, k2Pick;

extern vector<VECTOR3d> k, d;
extern Vec3f cy_p, plug_p, ocam_p, icam_p, wash_p, lp_p;
extern vector <Vec3f> key_p;
extern float keyangle;

float lastkeyangle;
int lastSelect;
float lastX, lastY;

extern vector<COLLISON> c_kpin, c_key, c_wkey;
extern COLLISON c_incam, c_lp, c_lph;

extern GLuint room, wh;

void init(void)
{
	glClearColor(0.8f, 0.8f, 0.8f, 0.75f);
	camInit();

	glEnable(GL_DEPTH_TEST);

	lightInit();

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	setTextureMapping(SILVER);
	setTextureMapping(GOLD);
	setTextureMapping(DSILVER);
	setTextureMapping(CSILVER);
	setTextureMapping(CGOLD);
	skyTextureMapping(ROOM);
	skyTextureMapping(WH);
	pickingInit();
	print_man();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void draw(void) // 기존 함수 변경
{
	GLfloat light_position[] = { 0.0, 0.0, 0.0, 1.0 };

	cam.x = radius * sinf(theta * PI / 180) * cosf(phi * PI / 180);
	cam.y = radius * sinf(phi * PI / 180);
	cam.z = radius * cosf(theta * PI / 180) * cosf(phi * PI / 180);

	sradius = 15;
	scam.x = sradius * sinf(theta * PI / 180) * cosf(phi * PI / 180);
	scam.y = sradius * sinf(phi * PI / 180);
	scam.z = sradius * cosf(theta * PI / 180) * cosf(phi * PI / 180);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, current_width / current_height, 1, 500);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, current_width, current_height);
	glLoadIdentity();
	gluLookAt(cam.x, cam.y, cam.z, 0, 0, 0, 0, cosf(phi * PI / 180), 0);

	move_lightpos(light_position, 0.0f, 50.0f, 0.0f);


	if (mode == PART_VIEWER)
	{
		draw_textureSkybox(wh);
		if (mPick) DrawPartView();
		else draw_full();
	}
	else if (mode == UNLOCK_PRO)
	{
		draw_textureSkybox(wh);

		if (!lock && keyangle == 180) draw_text("UNLOCK!", 1, 1, 0);
		else draw_text("LOCK!", 1, 1, 0);

		if (halfMode) draw_half();
		else draw_full();

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-g_nGLWidth / 60.0f, g_nGLWidth / 60.0f, -g_nGLHeight / 60.0f,
			g_nGLHeight / 60.0f, -100, 100);
		glMatrixMode(GL_MODELVIEW);
		glViewport(0, current_height * 3 / 4, current_width / 4, current_height / 4);
		glLoadIdentity();

		move_lightpos(light_position, 0.0f, 0.0f, 50.0f);

		if (halfMode) draw_full();
		else draw_half();

		draw_axis();
	}
	else if (mode == ESCAPE)
	{
		draw_textureSkybox(room);

		if (lpSelect != 1)
		{
			draw_room_obj();

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(-g_nGLWidth / 40.0f, g_nGLWidth / 40.0f, -g_nGLHeight / 40.0f,
				g_nGLHeight / 40.0f, -100, 100);
			glMatrixMode(GL_MODELVIEW);
			glViewport(0, 0, current_width, current_height);
			glLoadIdentity();

			move_lightpos(light_position, 0.0f, 0.0f, 50.0f);

			draw_item_box();

			draw_item();
			if (lpSelect == 2) draw_text("Door is Open!", 0, 1, 0);
			if (lpSelect == 3) draw_text("Wrong Key!", 0, 1, 0);
			//printf("%d\n", lpSelect);
		}
		else
		{
			sradius = 40;
			scam.x = sradius * sinf(stheta * PI / 180) * cosf(sphi * PI / 180);
			scam.y = sradius * sinf(sphi * PI / 180);
			scam.z = sradius * cosf(stheta * PI / 180) * cosf(sphi * PI / 180);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(45, current_width / current_height, 1, 500);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glMatrixMode(GL_MODELVIEW);
			glViewport(0, 0, current_width, current_height);
			glLoadIdentity();
			gluLookAt(scam.x, scam.y, scam.z, 0, 0, 0, 0, cosf(sphi * PI / 180), 0);

			move_lightpos(light_position, 50.0f, 0.0f, .0f);

			draw_lockpicking();

			if (lastkeyangle != keyangle && keyangle == 180)
			{
				PlaySound(TEXT("sound/unlock.wav"), NULL, SND_ASYNC);
			}
			lastkeyangle = keyangle;
		}
	}

	//draw_axis();

	glFlush();
	glutSwapBuffers(); // double buffering시
}

void mouse(int button, int state, int x, int y) {
	/* 인자들을 해석해서 원하는 기능을 구현 */
	//printf("%d, %d\n",button,state);
	//printf("%d\n", mouseBut);

	if (button == 0 && state == 0) mouseBut = GLUT_LEFT_BUTTON;
	else if (button == 1 && state == 0) mouseBut = GLUT_MIDDLE_BUTTON;

	y = g_nGLHeight - y;
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && !mPick && done && lpSelect != 1)
	{
		Picking(x, y);
		if (mode == ESCAPE)
		{
			lpSelect = 0;
			ItemPicking(x, y);

			if (lastSelect == 1 && g_nSelect == 7)
			{
				lpSelect = 1;
				camInit();
			}
			else if (lastSelect == 2 && g_nSelect == 7)
			{
				lpSelect = 2;
				PlaySound(TEXT("sound/unlock.wav"), NULL, SND_ASYNC);
			}
			else if (lastSelect == 3 && g_nSelect == 7)
			{
				lpSelect = 3;
				PlaySound(TEXT("sound/lock.wav"), NULL, SND_ASYNC);
			}
			//printf("%d\n", lpSelect);
		}

		g_nX = x;
		g_nY = y;

		lastSelect = g_nSelect;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && mode == PART_VIEWER && !mPick && done)
		g_nSelect = 0;
}

void motion(int x, int y)
{
	/* 인자들을 해석해서 원하는 기능을 구현 */
	if (mouseBut == GLUT_LEFT_BUTTON && !(mode == PART_VIEWER && !mPick) && lpSelect != 1) {
		if (x > lastX) {
			theta -= 1;
			if (theta <= 0) theta += 360;
		}
		else if (x < lastX) {
			theta += 1;
			if (theta > 360) theta -= 360;
		}
		if (y < lastY) {
			phi -= 1;
			if (phi <= 0) phi += 360;
		}
		else if (y > lastY) {
			phi += 1;
			if (phi > 360) phi -= 360;
		}
	}
	else if (lpSelect == 1) {
		if (x > lastX) {
			stheta -= 1;
			if (stheta <= 0) stheta += 360;
		}
		else if (x < lastX) {
			stheta += 1;
			if (stheta > 360) stheta -= 360;
		}
		if (y < lastY) {
			sphi -= 1;
			if (sphi <= 0) sphi += 360;
		}
		else if (y > lastY) {
			sphi += 1;
			if (sphi > 360) sphi -= 360;
		}
	}

	if (mouseBut == GLUT_LEFT_BUTTON && (mode == PART_VIEWER && !mPick) && done)
	{
		float px = x;
		float py = g_nGLHeight - y;
		if (g_nSelect > 0) {
			g_pPosition[g_nSelect - 1].x += ((float)px - g_nX) * 0.05;
			g_pPosition[g_nSelect - 1].y += ((float)py - g_nY) * 0.05;
			g_nX = px;
			g_nY = py;
			glutPostRedisplay();
		}
	}

	lastX = x; lastY = y;
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	/* 인자들을 해석해서 원하는 기능을 구현 */
	if (key == '0') lightMode = 0;
	else if (key == '1') lightMode = 1;
	else if (key == '2') lightMode = 2;
	else if (key == '3') lightMode = 3;
	else if (key == '4') lightMode = 4;
	else if (key == 'h') halfMode = !halfMode;
	else if (key == 'k' && keyangle == 0) keyMode = !keyMode;
	else if (key == 'z' && mode == PART_VIEWER)
	{
		camInit();
		mPick = false;
	}
	else if (key == 'x' && g_nSelect != 0) mPick = true;
	else if (key == 'c' && mode == PART_VIEWER && done && !mPick)
	{
		pickingInit();
		assemble = !assemble;
		done = false;
	}

	if (lpSelect == 1 && keyangle == 0 && lp_p.y <= 0)
	{
		if (key == 'd' && lp_p.z < 0 )
		{
			lp_p.z += 0.5;
			c_lp.position.z += 0.5;
			c_lph.position.z += 0.5;
		}
		else if (key == 'a' && lp_p.z > -10.5 )
		{
			lp_p.z -= 0.5;
			c_lp.position.z -= 0.5;
			c_lph.position.z -= 0.5;
		}

		if (key == 'w')
		{
			if (c_lp.position.z < -2.94 && c_lp.position.z > -2.96 && !pinlock[4]) hit = true;
			else if (c_lp.position.z < -1.95 && c_lp.position.z > -1.96 && !pinlock[3]) hit = true;
			else if (c_lp.position.z < -0.95 && c_lp.position.z > -0.96 && !pinlock[2]) hit = true;
			else if (c_lp.position.z < 0.05 && c_lp.position.z > 0.04 && !pinlock[1]) hit = true;
			else if (c_lp.position.z < 1.05 && c_lp.position.z > 1.04 && !pinlock[0]) hit = true;
		}
	}
}

void special(int key, int x, int y) {
	if (mode == UNLOCK_PRO)
	{
		if (key == GLUT_KEY_RIGHT && key_p[0].z < 10 && keyangle == 0 && keyMode)
		{
			key_p[0].z += 0.1;
			for (int i = 0; i < 6; i++)	c_key[i].position.z += 0.1;
		}
		else if (key == GLUT_KEY_LEFT && key_p[0].z > 0 && keyangle == 0 && keyMode)
		{
			key_p[0].z -= 0.1;
			for (int i = 0; i < 6; i++)	c_key[i].position.z -= 0.1;
		}

		if (key == GLUT_KEY_RIGHT && key_p[1].z < 10 && keyangle == 0 && !keyMode)
		{
			key_p[1].z += 0.1;
			for (int i = 0; i < 5; i++)	c_wkey[i].position.z += 0.1;
		}
		else if (key == GLUT_KEY_LEFT && key_p[1].z > 0 && keyangle == 0 && !keyMode)
		{
			key_p[1].z -= 0.1;
			for (int i = 0; i < 5; i++)	c_wkey[i].position.z -= 0.1;
		}

		if (key == GLUT_KEY_UP && keyangle < 180 && !lock)
		{
			keyangle += 5;
		}
		else if (key == GLUT_KEY_DOWN && keyangle > 0 && !lock)
		{
			keyangle -= 5;
		}
	}

	if (lpSelect == 1)
	{
		if (key == GLUT_KEY_UP)
		{

			if (c_lp.position.z < -2.94 && c_lp.position.z > -2.96 &&
				d[4].y >= 1.06 && d[4].y <= 1.11)
				pinlock[4] = true;	//1.11
			else if (c_lp.position.z < -1.95 && c_lp.position.z > -1.96 &&
				d[3].y > 0.31 && d[3].y <= 0.36)
				pinlock[3] = true;	//0.36
			else if (c_lp.position.z < -0.95 && c_lp.position.z > -0.96 &&
				d[2].y > 0.81 && d[2].y <= 0.86)
				pinlock[2] = true;	//0.86
			else if (c_lp.position.z < 0.05 && c_lp.position.z > 0.04 &&
				d[1].y > 0.56 && d[2].y <= 0.61)
				pinlock[1] = true;	//0.61
			else if (c_lp.position.z < 1.05 && c_lp.position.z > 1.04 &&
				d[0].y > 1.26 && d[2].y <= 1.31)
				pinlock[0] = true; //1.31
		}
		if (key == GLUT_KEY_UP && keyangle < 180 && !lock)
		{
			keyangle += 5;
		}
		else if (key == GLUT_KEY_DOWN && keyangle > 0 && !lock)
		{
			keyangle -= 5;
		}
	}
	glutPostRedisplay();
}

void mousewheel(int wheel, int direction, int x, int y)
{
	if (direction > 0 && radius > 1 && lpSelect != 1) radius -= 1;
	else if (direction < 0 && radius < 100)radius += 1;
	//printf("%f", radius);
	glutPostRedisplay();
}

void mode_menu_function(int option)
{
	if (option == 11)
	{
		mode = PART_VIEWER;
		pickingInit();
		camInit();
		g_nSelect = 0;
		mPick = false;
		done = true;
		step = 0;
		lpSelect = 0;
	}
	else if (option == 12)
	{
		mode = UNLOCK_PRO;
		unlockPosInit();
		camInit();
		mPick = false;
		lpSelect = 0;
	}
	else if (option == 13)
	{
		mode = ESCAPE;
		camInit();
		unlockPosInit();
		mPick = false;
		g_nSelect = 0;
		lpSelect = 0;
		lpPick = false;
		k1Pick = false;
		k2Pick = false;
		pinlock[0] = false;
		pinlock[1] = false;
		pinlock[2] = false;
		pinlock[3] = false;
		pinlock[4] = false;
	}
	glutPostRedisplay();
}

void main_menu_function(int option)
{
	if (option == 1) camInit();
	else if (option == 2)
	{
		if (mode == PART_VIEWER) pickingInit();
		else
		{
			unlockPosInit();
			lpPick = false;
			k1Pick = false;
			k2Pick = false;
			pinlock[0] = false;
			pinlock[1] = false;
			pinlock[2] = false;
			pinlock[3] = false;
			pinlock[4] = false;
		}
	}
	else if (option == 99) exit(1);
	glutPostRedisplay();
}


void idle(void)
{
	if (mode == UNLOCK_PRO)
	{
		key_pin_OBB();
	}

	else if (mode == PART_VIEWER && !done)
	{
		ass_disass();
	}

	else if (mode == ESCAPE && lpSelect == 1)
	{
		lp_pin_OBB();
	}

	//printf("%f, %f, %f, %f, %f, %f\n", d[0].y, d[1].y, d[2].y, d[3].y, d[4].y, icam_p.z);
	//if (!lock) printf("unlock\n");
	//printf("%lf\n", g_pPosition[3].x);
	glutPostRedisplay();
}

void resize(int width, int height)
{
	current_width = width; current_height = width;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (float)width / (float)height, 1, 500);	//인자 변경
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv)
{

	/* Window 초기화 */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	glutInitWindowPosition(200, 200);
	glutCreateWindow("3D Lock Catalog");
	init(); // -> 사용자 초기화 함수

	/* Popup menu 생성 및 추가 */
	int modeMenu;

	modeMenu = glutCreateMenu(mode_menu_function);
	glutAddMenuEntry("Part 3D Viewer", 11);
	glutAddMenuEntry("Unlock Process", 12);
	glutAddMenuEntry("Escape", 13);

	glutCreateMenu(main_menu_function);
	glutAddMenuEntry("Cam Init", 1);
	glutAddMenuEntry("Position Init", 2);
	glutAddSubMenu("Mode", modeMenu);
	glutAddMenuEntry("Quit", 99);

	glutAttachMenu(GLUT_RIGHT_BUTTON);

	/* Callback 함수 정의 */
	glutDisplayFunc(draw); /* draw() -> 실제 그리기 함수 */

	glutReshapeFunc(resize);

	glutMouseFunc(mouse);

	glutMotionFunc(motion);

	glutKeyboardFunc(keyboard);

	glutSpecialFunc(special);

	glutMouseWheelFunc(mousewheel);

	glutIdleFunc(idle);

	/* Looping 시작 */
	glutMainLoop();
	return 0;
}