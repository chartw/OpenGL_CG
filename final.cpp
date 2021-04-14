#include "final.h"

ObjParser* cy = new ObjParser("obj/cylinder.obj");
ObjParser* d0 = new ObjParser("obj/dpin0.obj");
ObjParser* d1 = new ObjParser("obj/dpin1.obj");
ObjParser* d2 = new ObjParser("obj/dpin2.obj");
ObjParser* d3 = new ObjParser("obj/dpin3.obj");
ObjParser* d4 = new ObjParser("obj/dpin4.obj");
ObjParser* h_cy = new ObjParser("obj/h_cylinder.obj");
ObjParser* h_ocam = new ObjParser("obj/h_out_cam.obj");
ObjParser* h_plug = new ObjParser("obj/h_plug.obj");
ObjParser* h_wash = new ObjParser("obj/h_washer.obj");
ObjParser* icam = new ObjParser("obj/in_cam.obj");
ObjParser* key1 = new ObjParser("obj/key.obj");
ObjParser* key2 = new ObjParser("obj/w_key.obj");
ObjParser* k0 = new ObjParser("obj/kpin0.obj");
ObjParser* k1 = new ObjParser("obj/kpin1.obj");
ObjParser* k2 = new ObjParser("obj/kpin2.obj");
ObjParser* k3 = new ObjParser("obj/kpin3.obj");
ObjParser* k4 = new ObjParser("obj/kpin4.obj");
ObjParser* ocam = new ObjParser("obj/out_cam.obj");
ObjParser* plug = new ObjParser("obj/plug.obj");
ObjParser* wash = new ObjParser("obj/washer.obj");
ObjParser* handle = new ObjParser("obj/lphandle.obj");
ObjParser* head = new ObjParser("obj/lphead.obj");

GLuint textureS;
GLuint textureG;
GLuint textureDS;
GLuint textureCS;
GLuint textureCG;
GLuint room;
GLuint wh;

int current_width, current_height;

#define N 2*17
vector<Vec2f> g_pPosition(N);
int g_nX, g_nY;
int g_nSelect = 0;
int g_nGLWidth = 800, g_nGLHeight = 800;

float PI = 3.14159265;

float radius,theta,phi;
float sradius, stheta, sphi;
Vec3f cam, scam;

bool keyMode = true;
int mode = 1;
bool mPick = false;
int itemPick = 0;
bool assemble = true;
bool lock = true;
bool done = true;
bool lpPick = false, k1Pick = false, k2Pick = false;
int step = 0;
int lpSelect = 0;
bool hit = false;
bool pinlock[5] = { false };

vector<VECTOR3d> k(5);
vector<VECTOR3d> d(5);
Vec3f cy_p, plug_p, ocam_p, icam_p, wash_p, lp_p;
vector <Vec3f> key_p(2);
float keyangle;
float hitangle;

vector<COLLISON> c_kpin(5);
vector<COLLISON> c_key(6);
vector<COLLISON> c_wkey(5);
COLLISON c_incam;
COLLISON c_lp, c_lph;

void move_lightpos(GLfloat* pos, GLfloat x, GLfloat y, GLfloat z)
{
	pos[0] = x;
	pos[1] = y;
	pos[2] = z;
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
}

void ItemPicking(int x, int y)
{
	GLuint selectBuf[100];
	// first step
	glSelectBuffer(100, selectBuf);
	// second step
	glRenderMode(GL_SELECT);
	// third step
	glMatrixMode(GL_PROJECTION);
	// third step
	glInitNames();
	glPushName(-1);

	glPushMatrix();
	glLoadIdentity();
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	gluPickMatrix(x, y, 0.01, 0.01, viewport);

	// fourth step

	glOrtho(-g_nGLWidth / 40.0f, g_nGLWidth / 40.0f, -g_nGLHeight / 40.0f,
		g_nGLHeight / 40.0f, -100, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();

	draw_item();

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glFlush();

	GLint hits = glRenderMode(GL_RENDER);
	// sixth step
	if (hits <= 0) return;
	//camInit();
	int stack = selectBuf[0];
	unsigned int zMin = selectBuf[1];
	unsigned int zMax = selectBuf[2];
	g_nSelect = selectBuf[3];
	int index = 3 + stack;
	int i;
	for (i = 1; i < hits; i++) {
		stack = selectBuf[index];
		if (zMin > selectBuf[index + 1]) {
			zMin = selectBuf[index + 1];
			//printf("@@%d\n", index + 2);
			g_nSelect = selectBuf[index + 3];
		}
		index += 3 + stack;
	}
	//printf("hits : %d\n", hits);
	//for (int i = 0; i < hits * 4; i++)
	//{
	//	printf("%u ", selectBuf[i]);
	//	if ((i + 1) % 4 == 0) printf("\n");
	//}
	//printf("g_nSelect : %d\n", g_nSelect);
	//printf("=======================================\n");
	//for(int n=0;n<N/2;n++) printf("%d : (%f, %f)\n", n+1, g_pPosition[2*n], g_pPosition[2*n+1]);
}

void Picking(int x, int y)
{
	GLuint selectBuf[100];
	// first step
	glSelectBuffer(100, selectBuf);
	// second step
	glRenderMode(GL_SELECT);
	// third step
	glMatrixMode(GL_PROJECTION);
	// third step
	glInitNames();
	glPushName(-1);

	glPushMatrix();
	glLoadIdentity();
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	gluPickMatrix(x, y, 0.01, 0.01, viewport);

	// fourth step

	gluPerspective(45, current_width / current_height, 1, 500);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(cam.x, cam.y, cam.z, 0, 0, 0, 0, cosf(phi * PI / 180), 0);
	glPushMatrix();

	if (mode == PART_VIEWER) draw_full();
	else if (mode == ESCAPE) draw_room_obj();

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glFlush();

	GLint hits = glRenderMode(GL_RENDER);
	// sixth step
	if (hits <= 0) return;
	//camInit();
	int stack = selectBuf[0];
	unsigned int zMin = selectBuf[1];
	unsigned int zMax = selectBuf[2];
	g_nSelect = selectBuf[3];
	int index = 3 + stack;
	int i;
	for (i = 1; i < hits; i++) {
		stack = selectBuf[index];
		if (zMin > selectBuf[index + 1]) {
			zMin = selectBuf[index + 1];
			//printf("@@%d\n", index + 2);
			g_nSelect = selectBuf[index + 3];
		}
		index += 3 + stack;
	}
	//printf("hits : %d\n", hits);
	//for (int i = 0; i < hits * 4; i++)
	//{
	//	printf("%u ", selectBuf[i]);
	//	if ((i + 1) % 4 == 0) printf("\n");
	//}
	//printf("g_nSelect : %d\n", g_nSelect);
	//printf("=======================================\n");
	//for(int n=0;n<N/2;n++) printf("%d : (%f, %f)\n", n+1, g_pPosition[2*n], g_pPosition[2*n+1]);
	if (mode == ESCAPE)
	{
		if (g_nSelect == 4)
		{
			lpPick = true;
			PlaySound(TEXT("sound/coin.wav"), NULL, SND_ASYNC);
		}
		if (g_nSelect == 5)
		{
			k1Pick = true;
			PlaySound(TEXT("sound/coin.wav"), NULL, SND_ASYNC);
		}
		if (g_nSelect == 6)
		{
			k2Pick = true;
			PlaySound(TEXT("sound/coin.wav"), NULL, SND_ASYNC);
		}

	}
}

void DrawPartView()
{
	glColor3f(1, 1, 1);
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	if (g_nSelect == 1)
	{
		glTranslatef(plug_p.x, plug_p.y, plug_p.z);
		draw_obj_tx(plug, textureCG);


	}
	else if (g_nSelect == 2)
	{
		glTranslatef(cy_p.x, cy_p.y, cy_p.z);
		draw_obj_tx(cy, textureCS);
	}
	else if (g_nSelect == 3)
	{
		glTranslatef(ocam_p.x, ocam_p.y, ocam_p.z);
		draw_obj_tx(ocam, textureDS);
	}
	else if (g_nSelect == 16)
	{
		glTranslatef(key_p[0].x, key_p[0].y, key_p[0].z);
		draw_obj_tx(key1, textureS);
	}
	else if (g_nSelect == 17)
	{
		glTranslatef(key_p[1].x, key_p[1].y, key_p[1].z);
		draw_obj_tx(key2, textureG);
	}
	else if (g_nSelect == 5)
	{
		glTranslatef(icam_p.x, icam_p.y, icam_p.z);
		draw_obj_tx(icam, textureS);
	}
	else if (g_nSelect == 4)
	{
		glTranslatef(wash_p.x, wash_p.y, wash_p.z);
		draw_obj_tx(wash, textureCG);
	}
	glDisable(GL_TEXTURE_2D);

	glRotatef(45, 0, 1, 0);
	glColor3f(1, 0, 0);
	if (g_nSelect == 6)
	{
		glTranslatef(k[0].x, k[0].y, k[0].z);
		draw_obj(k0);
	}
	else if (g_nSelect == 7)
	{
		glTranslatef(k[1].x, k[1].y, k[1].z);
		draw_obj(k1);
	}
	else if (g_nSelect == 8)
	{
		glTranslatef(k[2].x, k[2].y, k[2].z);
		draw_obj(k2);
	}
	else if (g_nSelect == 9)
	{
		glTranslatef(k[3].x, k[3].y, k[3].z);
		draw_obj(k3);
	}
	else if (g_nSelect == 10)
	{
		glTranslatef(k[4].x, k[4].y, k[4].z);
		draw_obj(k4);
	}

	glColor3f(0, 1, 0);
	if (g_nSelect == 11)
	{
		glTranslatef(d[0].x, d[0].y, d[0].z);
		draw_obj(d0);
	}
	else if (g_nSelect == 12)
	{
		glTranslatef(d[1].x, d[1].y, d[1].z);
		draw_obj(d1);
	}
	else if (g_nSelect == 13)
	{
		glTranslatef(d[2].x, d[2].y, d[2].z);
		draw_obj(d2);
	}
	else if (g_nSelect == 14)
	{
		glTranslatef(d[3].x, d[3].y, d[3].z);
		draw_obj(d3);
	}
	else if (g_nSelect == 15)
	{
		glTranslatef(d[4].x, d[4].y, d[4].z);
		draw_obj(d4);
	}
	glPopMatrix();
}

void key_pin_OBB()
{
	if (keyMode)
	{
		if (OBB(c_kpin[0], c_key[0]) || OBB(c_kpin[0], c_key[1]) || OBB(c_kpin[0], c_key[2])
			|| OBB(c_kpin[0], c_key[3]) || OBB(c_kpin[0], c_key[4])) {
			k[0].y += 0.02;
			d[0].y += 0.02;
			c_kpin[0].position.y += 0.02;
		}
		else if (k[0].y > 0.02) {
			k[0].y -= 0.02;
			d[0].y -= 0.02;
			c_kpin[0].position.y -= 0.02;
		}
		if (OBB(c_kpin[1], c_key[0]) || OBB(c_kpin[1], c_key[1])
			|| OBB(c_kpin[1], c_key[2]) || OBB(c_kpin[1], c_key[3])) {
			k[1].y += 0.02;
			d[1].y += 0.02;
			c_kpin[1].position.y += 0.02;
		}
		else if (k[1].y > -0.71) {
			k[1].y -= 0.02;
			d[1].y -= 0.02;
			c_kpin[1].position.y -= 0.02;
		}
		if (OBB(c_kpin[2], c_key[0]) || OBB(c_kpin[2], c_key[1]) || OBB(c_kpin[2], c_key[2])) {
			k[2].y += 0.02;
			d[2].y += 0.02;
			c_kpin[2].position.y += 0.02;
		}
		else if (k[2].y > -0.46) {
			k[2].y -= 0.02;
			d[2].y -= 0.02;
			c_kpin[2].position.y -= 0.02;
		}
		if (OBB(c_kpin[3], c_key[0]) || OBB(c_kpin[3], c_key[1])) {
			k[3].y += 0.02;
			d[3].y += 0.02;
			c_kpin[3].position.y += 0.02;
		}
		else if (k[3].y > -0.96) {
			k[3].y -= 0.02;
			d[3].y -= 0.02;
			c_kpin[3].position.y -= 0.02;
		}
		if (OBB(c_kpin[4], c_key[0])) {
			k[4].y += 0.02;
			d[4].y += 0.02;
			c_kpin[4].position.y += 0.02;
		}
		else if (k[4].y > -0.21) {
			k[4].y -= 0.02;
			d[4].y -= 0.02;
			c_kpin[4].position.y -= 0.02;
		}

		if (OBB(c_incam, c_key[5]) && mode == UNLOCK_PRO) {
			icam_p.z -= 0.03;
			c_incam.position.z -= 0.03;
		}
		else if (icam_p.z < 0.03 && mode == UNLOCK_PRO)
		{
			icam_p.z += 0.03;
			c_incam.position.z += 0.03;
		}
	}

	else
	{
		if (OBB(c_kpin[0], c_wkey[0]) || OBB(c_kpin[0], c_wkey[1])
			|| OBB(c_kpin[0], c_wkey[2]) || OBB(c_kpin[0], c_wkey[3])) {
			k[0].y += 0.02;
			d[0].y += 0.02;
			c_kpin[0].position.y += 0.02;
		}
		else if (k[0].y > 0.02) {
			k[0].y -= 0.02;
			d[0].y -= 0.02;
			c_kpin[0].position.y -= 0.02;
		}
		if (OBB(c_kpin[1], c_wkey[0]) || OBB(c_kpin[1], c_wkey[1]) || OBB(c_kpin[1], c_wkey[2])) {
			k[1].y += 0.02;
			d[1].y += 0.02;
			c_kpin[1].position.y += 0.02;
		}
		else if (k[1].y > -0.71) {
			k[1].y -= 0.02;
			d[1].y -= 0.02;
			c_kpin[1].position.y -= 0.02;
		}
		if (OBB(c_kpin[2], c_wkey[0]) || OBB(c_kpin[2], c_wkey[1]) || OBB(c_kpin[2], c_wkey[2])) {
			k[2].y += 0.02;
			d[2].y += 0.02;
			c_kpin[2].position.y += 0.02;
		}
		else if (k[2].y > -0.46) {
			k[2].y -= 0.02;
			d[2].y -= 0.02;
			c_kpin[2].position.y -= 0.02;
		}
		if (OBB(c_kpin[3], c_wkey[0]) || OBB(c_kpin[3], c_wkey[1])) {
			k[3].y += 0.02;
			d[3].y += 0.02;
			c_kpin[3].position.y += 0.02;
		}
		else if (k[3].y > -0.96) {
			k[3].y -= 0.02;
			d[3].y -= 0.02;
			c_kpin[3].position.y -= 0.02;
		}
		if (OBB(c_kpin[4], c_wkey[0])) {
			k[4].y += 0.02;
			d[4].y += 0.02;
			c_kpin[4].position.y += 0.02;
		}
		else if (k[4].y > -0.21) {
			k[4].y -= 0.02;
			d[4].y -= 0.02;
			c_kpin[4].position.y -= 0.02;
		}
		if (OBB(c_incam, c_wkey[4]) && mode == UNLOCK_PRO) {
			icam_p.z -= 0.03;
			c_incam.position.z -= 0.03;
		}
		else if (icam_p.z < 0.03 && mode == UNLOCK_PRO)
		{
			icam_p.z += 0.03;
			c_incam.position.z += 0.03;
		}
	}

	if (d[0].y >= -0.02 && d[0].y <= 0.03 &&
		d[1].y >= -0.02 && d[1].y <= 0.03 &&
		d[2].y >= -0.02 && d[2].y <= 0.03 &&
		d[3].y >= -0.02 && d[3].y <= 0.03 &&
		d[4].y >= -0.02 && d[4].y <= 0.03 &&
		icam_p.z >= -1.5 && icam_p.z <= -1.46)
	{
		lock = false;
	}
	else lock = true;
}


void lp_pin_OBB()
{
	if (hit)
	{
		if (lp_p.y < 1)
		{
			lp_p.y += 0.0025;
			c_lp.position.y += 0.0025;
			c_lph.position.y += 0.0025;
		}
		else hit = false;
	}
	else
	{
		if (lp_p.y > 0)
		{
			lp_p.y -= 0.0025;
			c_lp.position.y -= 0.0025;
			c_lph.position.y -= 0.0025;
		}
	}

	if (OBB(c_kpin[0], c_lp) || OBB(c_kpin[0], c_lph))
	{
		k[0].y += 0.01;
		if (!pinlock[0]) d[0].y += 0.01;
		c_kpin[0].position.y += 0.01;
	}
	else if (k[0].y > -0.02) {
		k[0].y -= 0.01;
		if (!pinlock[0]) d[0].y -= 0.01;
		c_kpin[0].position.y -= 0.01;
	}
	if (OBB(c_kpin[1], c_lp) || OBB(c_kpin[1], c_lph)) {
		k[1].y += 0.01;
		if (!pinlock[1]) d[1].y += 0.01;
		c_kpin[1].position.y += 0.01;
	}
	else if (k[1].y > -0.71) {
		k[1].y -= 0.01;
		if (!pinlock[1]) d[1].y -= 0.01;
		c_kpin[1].position.y -= 0.01;
	}
	if (OBB(c_kpin[2], c_lp) || OBB(c_kpin[2], c_lph)) {
		k[2].y += 0.01;
		if (!pinlock[2]) d[2].y += 0.01;
		c_kpin[2].position.y += 0.01;
	}
	else if (k[2].y > -0.46) {
		k[2].y -= 0.01;
		if (!pinlock[2]) d[2].y -= 0.01;
		c_kpin[2].position.y -= 0.01;
	}
	if (OBB(c_kpin[3], c_lp) || OBB(c_kpin[3], c_lph)) {
		k[3].y += 0.01;
		if (!pinlock[3]) d[3].y += 0.01;
		c_kpin[3].position.y += 0.01;
	}
	else if (k[3].y > -0.96) {
		k[3].y -= 0.01;
		if (!pinlock[3]) d[3].y -= 0.01;
		c_kpin[3].position.y -= 0.01;
	}
	if (OBB(c_kpin[4], c_lp) || OBB(c_kpin[4], c_lph)) {
		k[4].y += 0.01;
		if (!pinlock[4]) d[4].y += 0.01;
		c_kpin[4].position.y += 0.01;
	}
	else if (k[4].y > -0.21) {
		k[4].y -= 0.01;
		if (!pinlock[4]) d[4].y -= 0.01;
		c_kpin[4].position.y -= 0.01;
	}
	if (OBB(c_incam, c_lp)) {
		icam_p.z -= 0.01;
		c_incam.position.z -= 0.01;
	}
	else if (icam_p.z < 0)
	{
		icam_p.z += 0.01;
		c_incam.position.z += 0.01;
	}

	if (d[0].y > 0 &&
		d[1].y > 0 &&
		d[2].y > 0 &&
		d[3].y > 0 &&
		d[4].y > 0 &&
		icam_p.z < -1)
	{
		lock = false;
	}
	else lock = true;

	//printf("%f %f %f %f %f \n", d[0].y, d[1].y, d[2].y, d[3].y, d[4].y);
	//printf("%f, %f\n", c_lp.position.z, c_incam.position.z);
}



void ass_disass()
{
	if (!assemble)
	{
		if (step == 0)
		{
			if (g_pPosition[2].x > -20.0f) g_pPosition[2].x -= 0.05f;
			else if (g_pPosition[4].x > -15.0f) g_pPosition[4].x -= 0.05f;
			else if (g_pPosition[3].x > -12.0f) g_pPosition[3].x -= 0.05f;
			else step = 1;
		}
		else if (step == 1)
		{
			if (keyangle < 90) keyangle += 0.5;
			else if (g_pPosition[0].x < 11.0f)
			{
				g_pPosition[0].x += 0.05f;
				g_pPosition[5].x += 0.05f;
				g_pPosition[6].x += 0.05f;
				g_pPosition[7].x += 0.05f;
				g_pPosition[8].x += 0.05f;
				g_pPosition[9].x += 0.05f;
			}
			else step = 2;
		}
		else if (step == 2)
		{
			if (keyangle > 0) keyangle -= 0.5;
			else step = 3;
		}
		else if (step == 3)
		{
			if (g_pPosition[5].y < 5.0f)
			{
				g_pPosition[5].y += 0.05f;
				g_pPosition[6].y += 0.05f;
				g_pPosition[7].y += 0.05f;
				g_pPosition[8].y += 0.05f;
				g_pPosition[9].y += 0.05f;
			}
			else step = 4;
		}
		else if (step == 4)
		{
			if (g_pPosition[10].y > -3.0f)
			{
				g_pPosition[10].y -= 0.05f;
				g_pPosition[11].y -= 0.05f;
				g_pPosition[12].y -= 0.05f;
				g_pPosition[13].y -= 0.05f;
				g_pPosition[14].y -= 0.05f;
			}
			else if (g_pPosition[10].x > -10.0f)
			{
				g_pPosition[10].x -= 0.05f;
				g_pPosition[11].x -= 0.05f;
				g_pPosition[12].x -= 0.05f;
				g_pPosition[13].x -= 0.05f;
				g_pPosition[14].x -= 0.05f;
			}
			else step = 5;
		}
		else if (step == 5)
		{
			if (g_pPosition[10].y < 5.0f)
			{
				g_pPosition[10].y += 0.05f;
				g_pPosition[11].y += 0.05f;
				g_pPosition[12].y += 0.05f;
				g_pPosition[13].y += 0.05f;
				g_pPosition[14].y += 0.05f;
			}
			else
			{
				done = true;
				step = 0;
			}
		}
	}
	else
	{
		if (step == 0)
		{
			if (g_pPosition[10].y > -3.0f)
			{
				g_pPosition[10].y -= 0.05f;
				g_pPosition[11].y -= 0.05f;
				g_pPosition[12].y -= 0.05f;
				g_pPosition[13].y -= 0.05f;
				g_pPosition[14].y -= 0.05f;
			}
			else step = 1;
		}
		else if (step == 1)
		{
			if (g_pPosition[10].x < 0.0f)
			{
				g_pPosition[10].x += 0.05f;
				g_pPosition[11].x += 0.05f;
				g_pPosition[12].x += 0.05f;
				g_pPosition[13].x += 0.05f;
				g_pPosition[14].x += 0.05f;
			}
			else if (g_pPosition[10].y < 0.0f)
			{
				g_pPosition[10].y += 0.05f;
				g_pPosition[11].y += 0.05f;
				g_pPosition[12].y += 0.05f;
				g_pPosition[13].y += 0.05f;
				g_pPosition[14].y += 0.05f;
			}
			else step = 2;
		}
		else if (step == 2)
		{
			if (g_pPosition[5].y > 0.0f)
			{
				g_pPosition[5].y -= 0.05f;
				g_pPosition[6].y -= 0.05f;
				g_pPosition[7].y -= 0.05f;
				g_pPosition[8].y -= 0.05f;
				g_pPosition[9].y -= 0.05f;
			}
			else step = 3;
		}
		else if (step == 3)
		{
			if (keyangle < 90) keyangle += 0.5;
			else step = 4;
		}
		else if (step == 4)
		{
			if (g_pPosition[0].x > 0.0f)
			{
				g_pPosition[0].x -= 0.05f;
				g_pPosition[5].x -= 0.05f;
				g_pPosition[6].x -= 0.05f;
				g_pPosition[7].x -= 0.05f;
				g_pPosition[8].x -= 0.05f;
				g_pPosition[9].x -= 0.05f;
			}
			else if (keyangle > 0) keyangle -= 0.5;
			else step = 5;
		}
		else if (step == 5)
		{
			if (g_pPosition[3].x < 0.0f) g_pPosition[3].x += 0.05f;
			else if (g_pPosition[4].x < 0.0f) g_pPosition[4].x += 0.05f;
			else if (g_pPosition[2].x < 0.0f) g_pPosition[2].x += 0.05f;
			else
			{
				done = true;
				step = 0;
			}
		}
	}

}

void print_man()
{
	printf("\n==============================================================\n");
	printf("Part 3D View Mode\n");
	printf("==============================================================\n");
	printf("Key C :\t\t\t\t\tAssemble / Disassemble\n");
	printf("Mouse LB Click / Drag :\t\t\tPick / Move Object \n");
	printf("Key X While Picking Part :\t\tPart 3D ViewW\n");
	printf("Key Z in 3D View :\t\t\tReturn Picking\n");

	printf("\n==============================================================\n");
	printf("Unlock Process Mode\n");
	printf("==============================================================\n");
	printf("Key H :\t\t\t\tHalf Mode Or Full Mode\n");
	printf("Key K :\t\t\t\tChange Key\n");
	printf("Key ←/→ :\t\t\tTranslate Key\n");
	printf("Key ↑/↓ :\t\t\tUnlock / Lock\n");

	printf("\n==============================================================\n");
	printf("Escape Mode\n");
	printf("==============================================================\n");
	printf("Key W :\t\tHit Pin\n");
	printf("Key A/D :\tTranslate Lock Pick\n");
	printf("Key ↑ :\t Pin Lock\n");
	printf("Key ↑/↓ :\tUnlock / Lock\n");

	printf("\n==============================================================\n");
	printf("Sub Menu\n");
	printf("==============================================================\n");
	printf("Cam Init :\tInitialize Camera Position\n");
	printf("Position Init :\tInitialize Object Position\n");
}


void camInit()
{
	if (mode == PART_VIEWER)
	{
		radius = 40;
		theta = 45, phi = 0;
	}
	else if (mode == UNLOCK_PRO)
	{
		radius = 40;
		theta = 45, phi = 0;
	}
	else if (lpSelect != 0)
	{
		sradius = 40;
		stheta = 45, sphi = 0;
	}
	else if (mode == ESCAPE)
	{
		radius = 50;
		theta = -120, phi = 0;
	}

}

void unlockPosInit()
{
	hitangle = 0;
	keyangle = 0;
	key_p[0] = { 0,0,10 };
	key_p[1] = { 0,0,10 };
	k[0] = { 0,0,0 };
	k[1] = { 0, -0.7,0 };
	k[2] = { 0,-0.45,0 };
	k[3] = { 0,-0.95,0 };
	k[4] = { 0,-0.2,0 };
	d[0] = { 0,0,0 };
	d[1] = { 0,-0.7,0 };
	d[2] = { 0,-0.45,0 };
	d[3] = { 0,-0.95,0 };
	d[4] = { 0,-0.2,0 };
	icam_p = { 0,0,0 };
	lp_p = { 0,0,0 };


	c_kpin[0].position.y = 0.3; c_kpin[0].position.z = 1; c_kpin[0].scale.y = 0.424; c_kpin[0].scale.z = 0.424; c_kpin[0].rot = 45;
	c_kpin[1].position.y = 0.3; c_kpin[1].position.z = 0; c_kpin[1].scale.y = 0.424; c_kpin[1].scale.z = 0.424; c_kpin[1].rot = 45;
	c_kpin[2].position.y = 0.3; c_kpin[2].position.z = -1; c_kpin[2].scale.y = 0.424; c_kpin[2].scale.z = 0.424; c_kpin[2].rot = 45;
	c_kpin[3].position.y = 0.3; c_kpin[3].position.z = -2; c_kpin[3].scale.y = 0.424; c_kpin[3].scale.z = 0.424; c_kpin[3].rot = 45;
	c_kpin[4].position.y = 0.3; c_kpin[4].position.z = -3; c_kpin[4].scale.y = 0.424; c_kpin[4].scale.z = 0.424; c_kpin[4].rot = 45;

	c_key[0].position.z = 6.14; c_key[0].scale.y = 1.5; c_key[0].scale.z = 1.5; c_key[0].rot = 45;
	c_key[1].position.z = 7.875; c_key[1].scale.y = 1.53; c_key[1].scale.z = 1.53; c_key[1].rot = 45;
	c_key[2].position.z = 8.25; c_key[2].scale.y = 1.7; c_key[2].scale.z = 1.7; c_key[2].rot = 45;
	c_key[3].position.z = 9.625; c_key[3].scale.y = 1.53; c_key[3].scale.z = 1.53; c_key[3].rot = 45;
	c_key[4].position.z = 10.15; c_key[4].scale.y = 1.20; c_key[4].scale.z = 1.20; c_key[4].rot = 45;

	c_wkey[0].position.z = 6.47855; c_wkey[0].scale.y = 1.98006; c_wkey[0].scale.z = 1.98006; c_wkey[0].rot = 45;
	c_wkey[1].position.z = 7.25; c_wkey[1].scale.y = 1.5962; c_wkey[1].scale.z = 1.5962; c_wkey[1].rot = 45;
	c_wkey[2].position.z = 9.25; c_wkey[2].scale.y = 2.3033; c_wkey[2].scale.z = 2.3033; c_wkey[2].rot = 45;
	c_wkey[3].position.z = 10.25; c_wkey[3].scale.y = 1.596194; c_wkey[3].scale.z = 1.596194; c_wkey[3].rot = 45;

	c_incam.position.z = -4.5; c_incam.scale.y = 1; c_incam.scale.z = 0.5;
	c_key[5].position.z = 5.5; c_key[5].scale.y = 1; c_key[5].scale.z = 0.5;
	c_wkey[4].position.z = 5.5; c_wkey[4].scale.y = 1; c_wkey[4].scale.z = 0.5;
	c_lp.position.z = 5.05; c_lp.scale.y = 0.6; c_lp.scale.z = 0.2;
	c_lph.position.y = -0.35; c_lph.position.z = 7; c_lph.scale.y = 0.15; c_lph.scale.z = 4.85; c_lph.rot = 0;

}

void pickingInit()
{
	if (assemble)
	{
		g_pPosition[0] = { 0,0 };
		g_pPosition[1] = { 0,0 };
		g_pPosition[2] = { 0,0 };
		g_pPosition[3] = { 0,0 };
		g_pPosition[4] = { 0,0 };
		g_pPosition[5] = { 0,0 };
		g_pPosition[6] = { 0,0 };
		g_pPosition[7] = { 0,0 };
		g_pPosition[8] = { 0,0 };
		g_pPosition[9] = { 0,0 };
		g_pPosition[10] = { 0,0 };
		g_pPosition[11] = { 0,0 };
		g_pPosition[12] = { 0,0 };
		g_pPosition[13] = { 0,0 };
		g_pPosition[14] = { 0,0 };
		g_pPosition[15] = { 4,-9 };
		g_pPosition[16] = { -15,-9 };
	}
	else
	{
		g_pPosition[0] = { 11, 0 };
		g_pPosition[1] = { 0, 0 };
		g_pPosition[2] = { -20, 0 };
		g_pPosition[3] = { -12, 0 };
		g_pPosition[4] = { -15, 0 };
		g_pPosition[5] = { 11, 5 };
		g_pPosition[6] = { 11, 5 };
		g_pPosition[7] = { 11, 5 };
		g_pPosition[8] = { 11, 5 };
		g_pPosition[9] = { 11, 5 };
		g_pPosition[10] = { -10, 5 };
		g_pPosition[11] = { -10, 5 };
		g_pPosition[12] = { -10, 5 };
		g_pPosition[13] = { -10, 5 };
		g_pPosition[14] = { -10, 5 };
		g_pPosition[15] = { 4,-9 };
		g_pPosition[16] = { -15,-9 };
	}
	k[0] = { 0, -1.05,-1 };
	k[1] = { 0,-1.75,0 };
	k[2] = { 0,-1.5,1 };
	k[3] = { 0,-2,2 };
	k[4] = { 0,-1.25,3 };
	d[0] = { 0,-3.3375,-1 };
	d[1] = { 0,-3.3375,0 };
	d[2] = { 0,-3.3375,1 };
	d[3] = { 0,-3.3375,2 };
	d[4] = { 0,-3.3375,3 };

	cy_p = { 0,0,0 };
	plug_p = { 0,0,0 };
	ocam_p = { 0,0,5.1 };
	icam_p = { 0,0,4.5 };
	wash_p = { 0,0,4.05 };

	key_p[0] = { 0,0, -4.5 };
	key_p[1] = { 0,0, -4.5 };
	keyangle = 0;
}

void draw_item_box()
{
	glColor3f(1.0f, 1.0f, 1.0f);
	if (g_nSelect == 1) {
		glBegin(GL_QUADS);
		glNormal3f(0, 0, 1);
		glVertex3f(g_nGLWidth / 80.0f, -g_nGLHeight / 80.0f, -30);
		glVertex3f(g_nGLWidth / 40.0f, -g_nGLHeight / 80.0f, -30);
		glVertex3f(g_nGLWidth / 40.0f, -g_nGLHeight / 40.0f, -30);
		glVertex3f(g_nGLWidth / 80.0f, -g_nGLHeight / 40.0f, -30);
		glEnd();
	}
	else if (g_nSelect == 2) {
		glBegin(GL_QUADS);
		glNormal3f(0, 0, 1);
		glVertex3f(0, -g_nGLHeight / 80.0f, -30);
		glVertex3f(g_nGLWidth / 80.0f, -g_nGLHeight / 80.0f, -30);
		glVertex3f(g_nGLWidth / 80.0f, -g_nGLHeight / 52.5f, -30);
		glVertex3f(0, -g_nGLHeight / 52.5f, -30);
		glEnd();
	}
	else if (g_nSelect == 3) {
		glBegin(GL_QUADS);
		glNormal3f(0, 0, 1);
		glVertex3f(0, -g_nGLHeight / 52.5f, -30);
		glVertex3f(g_nGLWidth / 80.0f, -g_nGLHeight / 52.5f, -30);
		glVertex3f(g_nGLWidth / 80.0f, -g_nGLHeight / 40.0f, -30);
		glVertex3f(0, -g_nGLHeight / 40.0f, -30);
		glEnd();
	}


	glColor3f(0.5f, 0.5f, 0.5f);
	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glVertex3f(0, -g_nGLHeight / 80.0f, -50);
	glVertex3f(g_nGLWidth / 40.0f, -g_nGLHeight / 80.0f, -50);
	glVertex3f(g_nGLWidth / 40.0f, -g_nGLHeight / 40.0f, -50);
	glVertex3f(0, -g_nGLHeight / 40.0f, -50);
	glEnd();
}

void draw_item()
{
	glColor3f(1, 1, 1);
	glEnable(GL_TEXTURE_2D);

	if (lpPick)
	{
		glLoadName(1);
		glPushMatrix();
		glScalef(0.4, 0.4, 0.4);
		glRotatef(90, 0, 1, 0);
		glTranslatef(0, -25, 25);
		glRotatef(45, 1, 0, 0);
		glColor3f(0, 0, 0);
		draw_obj(handle);

		glColor3f(1, 1, 1);
		draw_obj_tx(head, textureCS);
		glPopMatrix();
	}

	if (k1Pick)
	{
		glLoadName(2);
		glPushMatrix();
		glScalef(0.4, 0.4, 0.4);
		glRotatef(90, 0, 1, 0);
		glTranslatef(0, -32, 10);
		draw_obj_tx(key1, textureS);
		glPopMatrix();
	}

	if (k2Pick)
	{
		glLoadName(3);
		glPushMatrix();
		glScalef(0.4, 0.4, 0.4);
		glRotatef(90, 0, 1, 0);
		glTranslatef(0, -44, 10);
		draw_obj_tx(key2, textureG);
		glPopMatrix();
	}

	glDisable(GL_TEXTURE_2D);
}


void draw_lockpick()
{
	glPushMatrix();

	if (lpSelect == 1)
	{
		glTranslatef(lp_p.x, lp_p.y, lp_p.z);
		glRotatef(hitangle, 1, 0, 0);
		glRotatef(keyangle, 0, 0, 1);
	}
	else if (!lpPick)
	{
		glScalef(2, 2, 2);
		glTranslatef(90, -75, 50);
		glRotatef(90, 0, 0, 1);
	}

	glColor3f(0, 0, 0);
	draw_obj(handle);

	glEnable(GL_TEXTURE_2D);
	glColor3f(1, 1, 1);
	draw_obj_tx(head, textureCS);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

}

void draw_lockpicking()
{
	draw_half();
}

void draw_room_obj()
{
	glEnable(GL_BLEND);
	glColor4f(1, 1, 1, 0);
	glLoadName(7);
	glPushMatrix();

	glBegin(GL_QUADS);
	glVertex3f(-199, -68, 180);
	glVertex3f(-190, -80, 100);

	glVertex3f(-195, 75, 100);
	glVertex3f(-190, 110, 180);
	glEnd();
	glPopMatrix();
	glDisable(GL_BLEND);

	if (!lpPick)
	{
		glLoadName(4);
		glPushMatrix();

		glScalef(2, 2, 2);
		glTranslatef(90, -75, 50);
		glRotatef(90, 0, 0, 1);

		glColor3f(0, 0, 0);
		draw_obj(handle);

		glColor3f(1, 1, 1);
		glEnable(GL_TEXTURE_2D);
		draw_obj_tx(head, textureCS);
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);
	}

	glEnable(GL_TEXTURE_2D);
	if (!k1Pick)
	{
		glLoadName(5);
		glPushMatrix();

		glRotatef(90, 0, 1, 0);
		glScalef(0.5, 0.5, 0.5);
		glTranslatef(380, -120, -160);
		glRotatef(90, 0, 0, 1);

		glColor3f(1, 1, 1);
		draw_obj_tx(key1, textureS);
		glPopMatrix();
	}
	if (!k2Pick)
	{
		glLoadName(6);
		glPushMatrix();

		glScalef(0.5, 0.5, 0.5);
		glRotatef(90, 0, 1, 0);
		glTranslatef(-300, -150, -100);
		glRotatef(90, 0, 0, 1);

		glColor3f(1, 1, 1);
		draw_obj_tx(key2, textureG);
		glPopMatrix();
	}
	glDisable(GL_TEXTURE_2D);

}

void draw_key()
{
	glColor3f(1, 1, 1);
	glEnable(GL_TEXTURE_2D);

	if (mode == UNLOCK_PRO || mode == ESCAPE)
	{
		glPushMatrix();
		if (keyMode) glTranslatef(0, 0, key_p[0].z);
		else if (!keyMode) glTranslatef(0, 0, key_p[1].z);

		glRotatef(keyangle, 0, 0, 1);

		if (keyMode) draw_obj_tx(key1, textureS);
		else if (!keyMode) draw_obj_tx(key2, textureG);
		glPopMatrix();
	}
	else
	{
		glLoadName(16);
		glPushMatrix();
		glTranslatef(0.0f, g_pPosition[15].y, g_pPosition[15].x);
		draw_obj_tx(key1, textureS);
		glPopMatrix();

		glLoadName(17);
		glPushMatrix();
		glTranslatef(0.0f, g_pPosition[16].y, g_pPosition[16].x);
		draw_obj_tx(key2, textureG);
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);
	}


	glDisable(GL_TEXTURE_2D);
}

void draw_kpin()
{
	glColor3f(1, 0, 0);

	glLoadName(6);
	glPushMatrix();
	if (mode == UNLOCK_PRO || lpSelect == 1) glTranslatef(k[0].x, k[0].y, k[0].z);
	else glTranslatef(0.0f, g_pPosition[5].y, g_pPosition[5].x);
	draw_obj(k0);
	glPopMatrix();

	glLoadName(7);
	glPushMatrix();
	if (mode == UNLOCK_PRO || lpSelect == 1) glTranslatef(k[1].x, k[1].y, k[1].z);
	else glTranslatef(0.0f, g_pPosition[6].y, g_pPosition[6].x);
	draw_obj(k1);
	glPopMatrix();

	glLoadName(8);
	glPushMatrix();
	if (mode == UNLOCK_PRO || lpSelect == 1) glTranslatef(k[2].x, k[2].y, k[2].z);
	else glTranslatef(0.0f, g_pPosition[7].y, g_pPosition[7].x);
	draw_obj(k2);
	glPopMatrix();

	glLoadName(9);
	glPushMatrix();
	if (mode == UNLOCK_PRO || lpSelect == 1) glTranslatef(k[3].x, k[3].y, k[3].z);
	else glTranslatef(0.0f, g_pPosition[8].y, g_pPosition[8].x);
	draw_obj(k3);
	glPopMatrix();

	glLoadName(10);
	glPushMatrix();
	if (mode == UNLOCK_PRO || lpSelect == 1) glTranslatef(k[4].x, k[4].y, k[4].z);
	else glTranslatef(0.0f, g_pPosition[9].y, g_pPosition[9].x);
	draw_obj(k4);
	glPopMatrix();
}

void draw_dpin()
{
	glColor3f(0, 1, 0);
	glPushMatrix();

	glLoadName(11);
	glPushMatrix();
	if (mode == UNLOCK_PRO || lpSelect == 1) glTranslatef(d[0].x, d[0].y, d[0].z);
	else glTranslatef(0.0f, g_pPosition[10].y, g_pPosition[10].x);
	draw_obj(d0);
	glPopMatrix();

	glLoadName(12);
	glPushMatrix();
	if (mode == UNLOCK_PRO || lpSelect == 1) glTranslatef(d[1].x, d[1].y, d[1].z);
	else glTranslatef(0.0f, g_pPosition[11].y, g_pPosition[11].x);
	draw_obj(d1);
	glPopMatrix();

	glLoadName(13);
	glPushMatrix();
	if (mode == UNLOCK_PRO || lpSelect == 1) glTranslatef(d[2].x, d[2].y, d[2].z);
	else glTranslatef(0.0f, g_pPosition[12].y, g_pPosition[12].x);
	draw_obj(d2);
	glPopMatrix();

	glLoadName(14);
	glPushMatrix();
	if (mode == UNLOCK_PRO || lpSelect == 1) glTranslatef(d[3].x, d[3].y, d[3].z);
	else glTranslatef(0.0f, g_pPosition[13].y, g_pPosition[13].x);
	draw_obj(d3);
	glPopMatrix();

	glLoadName(15);
	glPushMatrix();
	if (mode == UNLOCK_PRO || lpSelect == 1) glTranslatef(d[4].x, d[4].y, d[4].z);
	else glTranslatef(0.0f, g_pPosition[14].y, g_pPosition[14].x);
	draw_obj(d4);
	glPopMatrix();

	glPopMatrix();
}

void draw_incam()
{
	glColor3f(1, 1, 1);
	glEnable(GL_TEXTURE_2D);

	glLoadName(5);
	glPushMatrix();
	if (mode == UNLOCK_PRO || lpSelect == 1)
	{
		glTranslatef(icam_p.x, icam_p.y, icam_p.z);
		glRotatef(keyangle, 0, 0, 1);
	}
	else glTranslatef(0.0f, g_pPosition[4].y, g_pPosition[4].x);
	draw_obj_tx(icam, textureS);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}


void draw_full()
{
	glPushMatrix();
	glRotatef(90, 0, 1, 0);

	draw_key();

	draw_dpin();

	draw_incam();

	glPushMatrix();
	glRotatef(keyangle, 0, 0, 1);
	glEnable(GL_TEXTURE_2D);
	glLoadName(1);
	glPushMatrix();

	if (mode == PART_VIEWER) glTranslatef(0.0f, g_pPosition[0].y, g_pPosition[0].x);
	draw_obj_tx(plug, textureCG);
	glPopMatrix();

	if (mode == PART_VIEWER)
	{
		glPopMatrix();
		glPushMatrix();
	}

	glLoadName(3);
	glPushMatrix();
	if (mode == PART_VIEWER) glTranslatef(0.0f, g_pPosition[2].y, g_pPosition[2].x);
	draw_obj_tx(ocam, textureDS);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);

	glRotatef(keyangle, 0, 0, 1);
	draw_kpin();
	glPopMatrix();

	glEnable(GL_TEXTURE_2D);

	glColor3f(1, 1, 1);
	glLoadName(2);
	glPushMatrix();
	if (mode == PART_VIEWER) glTranslatef(0.0f, g_pPosition[1].y, g_pPosition[1].x);
	draw_obj_tx(cy, textureCS);
	glPopMatrix();

	glLoadName(4);
	glPushMatrix();
	if (mode == PART_VIEWER) glTranslatef(0.0f, g_pPosition[3].y, g_pPosition[3].x);
	draw_obj_tx(wash, textureCG);
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void draw_half()
{
	glPushMatrix();
	glRotatef(90, 0, 1, 0);

	if (mode == UNLOCK_PRO) draw_key();
	else if (mode == ESCAPE) draw_lockpick();

	draw_dpin();

	draw_incam();

	glPushMatrix();
	glRotatef(keyangle, 0, 0, 1);

	glEnable(GL_TEXTURE_2D);
	draw_obj_tx(h_plug, textureCG);
	draw_obj_tx(h_ocam, textureDS);
	glDisable(GL_TEXTURE_2D);

	draw_kpin();

	glPopMatrix();

	glColor3f(1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	draw_obj_tx(h_cy, textureCS);
	draw_obj_tx(h_wash, textureCG);
	glDisable(GL_TEXTURE_2D);

	glPopMatrix();
}

void draw_obj(ObjParser* objParser)
{
	glBegin(GL_TRIANGLES);
	for (unsigned int n = 0; n < objParser->getFaceSize(); n += 3) {
		glNormal3f(objParser->normal[objParser->normalIdx[n] - 1].x,
			objParser->normal[objParser->normalIdx[n] - 1].y,
			objParser->normal[objParser->normalIdx[n] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n] - 1].x,
			objParser->vertices[objParser->vertexIdx[n] - 1].y,
			objParser->vertices[objParser->vertexIdx[n] - 1].z);

		glNormal3f(objParser->normal[objParser->normalIdx[n + 1] - 1].x,
			objParser->normal[objParser->normalIdx[n + 1] - 1].y,
			objParser->normal[objParser->normalIdx[n + 1] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n + 1] - 1].x,
			objParser->vertices[objParser->vertexIdx[n + 1] - 1].y,
			objParser->vertices[objParser->vertexIdx[n + 1] - 1].z);

		glNormal3f(objParser->normal[objParser->normalIdx[n + 2] - 1].x,
			objParser->normal[objParser->normalIdx[n + 2] - 1].y,
			objParser->normal[objParser->normalIdx[n + 2] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n + 2] - 1].x,
			objParser->vertices[objParser->vertexIdx[n + 2] - 1].y,
			objParser->vertices[objParser->vertexIdx[n + 2] - 1].z);
	}
	glEnd();
}

void setTextureMapping(GLuint texture) {
	int imgWidth, imgHeight, channels;
	int texNum = 1;
	uchar* img;

	if (texture == SILVER)
	{
		img = readImageData("tx/silver.bmp", &imgWidth, &imgHeight, &channels);
		glGenTextures(texNum, &textureS);
		glBindTexture(GL_TEXTURE_2D, textureS);
	}
	else if (texture == GOLD)
	{
		img = readImageData("tx/gold.bmp", &imgWidth, &imgHeight, &channels);
		glGenTextures(texNum, &textureG);
		glBindTexture(GL_TEXTURE_2D, textureG);
	}
	else if (texture == DSILVER)
	{
		img = readImageData("tx/d_silver.bmp", &imgWidth, &imgHeight, &channels);
		glGenTextures(texNum, &textureDS);
		glBindTexture(GL_TEXTURE_2D, textureDS);
	}
	else if (texture == CSILVER)
	{
		img = readImageData("tx/c_silver.bmp", &imgWidth, &imgHeight, &channels);
		glGenTextures(texNum, &textureCS);
		glBindTexture(GL_TEXTURE_2D, textureCS);
	}
	else if (texture == CGOLD)
	{
		img = readImageData("tx/c_gold.bmp", &imgWidth, &imgHeight, &channels);
		glGenTextures(texNum, &textureCG);
		glBindTexture(GL_TEXTURE_2D, textureCG);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, 3, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	//GL_REPEAT 둘중 하나 선택
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//gluBuild2DMipmaps(GL_TEXTURE_2D, 3, imgWidth, imgHeight, GL_RGB, GL_UNSIGNED_BYTE, img);
}


void draw_obj_tx(ObjParser* objParser, GLuint texture)
{
	glDisable(GL_BLEND);
	// glEnable(GL_TEXTURE_2D);	// texture 색 보존을 위한 enable
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_TRIANGLES);
	for (unsigned int n = 0; n < objParser->getFaceSize(); n += 3) {
		glTexCoord2f(objParser->textures[objParser->textureIdx[n] - 1].x,
			objParser->textures[objParser->textureIdx[n] - 1].y);
		glNormal3f(objParser->normal[objParser->normalIdx[n] - 1].x,
			objParser->normal[objParser->normalIdx[n] - 1].y,
			objParser->normal[objParser->normalIdx[n] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n] - 1].x,
			objParser->vertices[objParser->vertexIdx[n] - 1].y,
			objParser->vertices[objParser->vertexIdx[n] - 1].z);

		glTexCoord2f(objParser->textures[objParser->textureIdx[n + 1] - 1].x,
			objParser->textures[objParser->textureIdx[n + 1] - 1].y);
		glNormal3f(objParser->normal[objParser->normalIdx[n + 1] - 1].x,
			objParser->normal[objParser->normalIdx[n + 1] - 1].y,
			objParser->normal[objParser->normalIdx[n + 1] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n + 1] - 1].x,
			objParser->vertices[objParser->vertexIdx[n + 1] - 1].y,
			objParser->vertices[objParser->vertexIdx[n + 1] - 1].z);

		glTexCoord2f(objParser->textures[objParser->textureIdx[n + 2] - 1].x,
			objParser->textures[objParser->textureIdx[n + 2] - 1].y);
		glNormal3f(objParser->normal[objParser->normalIdx[n + 2] - 1].x,
			objParser->normal[objParser->normalIdx[n + 2] - 1].y,
			objParser->normal[objParser->normalIdx[n + 2] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n + 2] - 1].x,
			objParser->vertices[objParser->vertexIdx[n + 2] - 1].y,
			objParser->vertices[objParser->vertexIdx[n + 2] - 1].z);
	}
	glEnd();
	glEnable(GL_BLEND);
}

void draw_textureSkybox(GLuint g_nCubeTex)
{
	glColor3f(1, 1, 1);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, g_nCubeTex);
	float g_nSkySize = 200.0f;
	glBegin(GL_QUADS);

	//px
	glTexCoord3d(1.0, -1.0, -1.0); glVertex3f(g_nSkySize, -g_nSkySize, -g_nSkySize);
	glTexCoord3d(1.0, -1.0, 1.0); glVertex3f(g_nSkySize, -g_nSkySize, g_nSkySize);
	glTexCoord3d(1.0, 1.0, 1.0); glVertex3f(g_nSkySize, g_nSkySize, g_nSkySize);
	glTexCoord3d(1.0, 1.0, -1.0); glVertex3f(g_nSkySize, g_nSkySize, -g_nSkySize);

	//nx
	glTexCoord3d(-1.0, -1.0, -1.0); glVertex3f(-g_nSkySize, -g_nSkySize, -g_nSkySize);
	glTexCoord3d(-1.0, -1.0, 1.0); glVertex3f(-g_nSkySize, -g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, 1.0, 1.0); glVertex3f(-g_nSkySize, g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, 1.0, -1.0); glVertex3f(-g_nSkySize, g_nSkySize, -g_nSkySize);

	//py
	glTexCoord3d(1.0, 1.0, 1.0); glVertex3f(g_nSkySize, g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, 1.0, 1.0); glVertex3f(-g_nSkySize, g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, 1.0, -1.0); glVertex3f(-g_nSkySize, g_nSkySize, -g_nSkySize);
	glTexCoord3d(1.0, 1.0, -1.0); glVertex3f(g_nSkySize, g_nSkySize, -g_nSkySize);

	//ny
	glTexCoord3d(1.0, -1.0, 1.0); glVertex3f(g_nSkySize, -g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, -1.0, 1.0); glVertex3f(-g_nSkySize, -g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, -1.0, -1.0); glVertex3f(-g_nSkySize, -g_nSkySize, -g_nSkySize);
	glTexCoord3d(1.0, -1.0, -1.0); glVertex3f(g_nSkySize, -g_nSkySize, -g_nSkySize);

	//pz
	glTexCoord3d(1.0, -1.0, 1.0); glVertex3f(g_nSkySize, -g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, -1.0, 1.0); glVertex3f(-g_nSkySize, -g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, 1.0, 1.0); glVertex3f(-g_nSkySize, g_nSkySize, g_nSkySize);
	glTexCoord3d(1.0, 1.0, 1.0); glVertex3f(g_nSkySize, g_nSkySize, g_nSkySize);

	//nz
	glTexCoord3d(1.0, -1.0, -1.0); glVertex3f(g_nSkySize, -g_nSkySize, -g_nSkySize);
	glTexCoord3d(-1.0, -1.0, -1.0); glVertex3f(-g_nSkySize, -g_nSkySize, -g_nSkySize);
	glTexCoord3d(-1.0, 1.0, -1.0); glVertex3f(-g_nSkySize, g_nSkySize, -g_nSkySize);
	glTexCoord3d(1.0, 1.0, -1.0); glVertex3f(g_nSkySize, g_nSkySize, -g_nSkySize);

	glEnd();
	glEnable(GL_LIGHTING);
	glDisable(GL_TEXTURE_CUBE_MAP);
}

void skyTextureMapping(int skytex)
{
	int imgWidth, imgHeight, channels;
	// 바인딩

	if (skytex == ROOM)
	{
		glGenTextures(1, &room);
		glBindTexture(GL_TEXTURE_CUBE_MAP, room);
		// 각 영상들은 각 위치에 지정
		uchar* img0 = readImageData("skybox/room/px.bmp", &imgWidth, &imgHeight, &channels);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, /*INPUT CHANNEL*/3, imgWidth, imgHeight, 0, /*TEXEL CHANNEL*/GL_RGB, GL_UNSIGNED_BYTE, img0);
		uchar* img1 = readImageData("skybox/room/nx.bmp", &imgWidth, &imgHeight, &channels);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, /*INPUT CHANNEL*/3, imgWidth, imgHeight, 0, /*TEXEL CHANNEL*/GL_RGB, GL_UNSIGNED_BYTE, img1);
		uchar* img2 = readImageData("skybox/room/py.bmp", &imgWidth, &imgHeight, &channels);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, /*INPUT CHANNEL*/3, imgWidth, imgHeight, 0, /*TEXEL CHANNEL*/GL_RGB, GL_UNSIGNED_BYTE, img2);
		uchar* img3 = readImageData("skybox/room/ny.bmp", &imgWidth, &imgHeight, &channels);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, /*INPUT CHANNEL*/3, imgWidth, imgHeight, 0, /*TEXEL CHANNEL*/GL_RGB, GL_UNSIGNED_BYTE, img3);
		uchar* img4 = readImageData("skybox/room/nz.bmp", &imgWidth, &imgHeight, &channels);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, /*INPUT CHANNEL*/3, imgWidth, imgHeight, 0, /*TEXEL CHANNEL*/GL_RGB, GL_UNSIGNED_BYTE, img4);
		uchar* img5 = readImageData("skybox/room/pz.bmp", &imgWidth, &imgHeight, &channels);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, /*INPUT CHANNEL*/3, imgWidth, imgHeight, 0, /*TEXEL CHANNEL*/GL_RGB, GL_UNSIGNED_BYTE, img5);
		// 각 텍스쳐 좌표의 특성을 지정
		glBindTexture(GL_TEXTURE_CUBE_MAP, room);
	}
	else if (skytex == WH)
	{
		glGenTextures(1, &wh);
		glBindTexture(GL_TEXTURE_CUBE_MAP, wh);
		// 각 영상들은 각 위치에 지정
		uchar* img0 = readImageData("skybox/wh/px.bmp", &imgWidth, &imgHeight, &channels);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, /*INPUT CHANNEL*/3, imgWidth, imgHeight, 0, /*TEXEL CHANNEL*/GL_RGB, GL_UNSIGNED_BYTE, img0);
		uchar* img1 = readImageData("skybox/wh/nx.bmp", &imgWidth, &imgHeight, &channels);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, /*INPUT CHANNEL*/3, imgWidth, imgHeight, 0, /*TEXEL CHANNEL*/GL_RGB, GL_UNSIGNED_BYTE, img1);
		uchar* img2 = readImageData("skybox/wh/py.bmp", &imgWidth, &imgHeight, &channels);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, /*INPUT CHANNEL*/3, imgWidth, imgHeight, 0, /*TEXEL CHANNEL*/GL_RGB, GL_UNSIGNED_BYTE, img2);
		uchar* img3 = readImageData("skybox/wh/ny.bmp", &imgWidth, &imgHeight, &channels);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, /*INPUT CHANNEL*/3, imgWidth, imgHeight, 0, /*TEXEL CHANNEL*/GL_RGB, GL_UNSIGNED_BYTE, img3);
		uchar* img4 = readImageData("skybox/wh/nz.bmp", &imgWidth, &imgHeight, &channels);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, /*INPUT CHANNEL*/3, imgWidth, imgHeight, 0, /*TEXEL CHANNEL*/GL_RGB, GL_UNSIGNED_BYTE, img4);
		uchar* img5 = readImageData("skybox/wh/pz.bmp", &imgWidth, &imgHeight, &channels);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, /*INPUT CHANNEL*/3, imgWidth, imgHeight, 0, /*TEXEL CHANNEL*/GL_RGB, GL_UNSIGNED_BYTE, img5);
		// 각 텍스쳐 좌표의 특성을 지정
		glBindTexture(GL_TEXTURE_CUBE_MAP, wh);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// 텍스쳐 좌표설정
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
}

void lightInit()
{
	// 0번 조명 관련 설정

	GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	//glLightfv(GL_LIGHT1, GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

	// 조명 스위치와 0번 조명 스위치 켜기
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
}

void draw_axis(void)
{
	sradius = 10;

	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glViewport(current_width * 9 / 10, current_height * 9 / 10, current_width / 10, current_height / 10);
	glLoadIdentity();
	gluLookAt(scam.x, scam.y, scam.z, 0, 0, 0, 0, cos(phi * PI / 180), 0);

	glLineWidth(3); // 좌표축의 두께
	glBegin(GL_LINES);
	glColor3f(1, 0, 0); // X축은 red
	glVertex3f(0, 0, 0);
	glVertex3f(10, 0, 0);

	glColor3f(0, 1, 0); // Y축은 green
	glVertex3f(0, 0, 0);
	glVertex3f(0, 10, 0);

	glColor3f(0, 0, 1); // Z축은 blue
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 10);
	glEnd();
	glLineWidth(1); // 두께 다시 환원

	glFlush();
	glPopAttrib();
}


void draw_string(void* font, const char* str, float x, float y, float red, float green, float blue)
{
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(-10, 10, -10, 10);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glColor3f(red, green, blue);
	glRasterPos3f(x, y, 0);
	for (unsigned int i = 0; i < strlen(str); i++) {
		glutBitmapCharacter(font, str[i]);
	}
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopAttrib();
}

void draw_text(const char* str, int r, int g, int b)
{
	if (mode == UNLOCK_PRO) draw_string(GLUT_BITMAP_HELVETICA_18, str, 0, 9, r, g, b);
	else if (mode == ESCAPE) draw_string(GLUT_BITMAP_HELVETICA_18, str, 0, 0, r, g, b);

	glFlush();
}


double absDotVector(VECTOR3d a, VECTOR3d b) { //vector inner
	return abs(a.z * b.z + a.y * b.y);
}

double Deg2Rad(double deg) { //deg -> rad
	return deg / 180 * 3.141592;
}

VECTOR3d getDistanceVector(COLLISON a, COLLISON b) { //distance vector
	VECTOR3d ret;
	ret.z = a.position.z - b.position.z;
	ret.y = a.position.y - b.position.y;
	return ret;
}

VECTOR3d getHeightVector(COLLISON a) { //height vector
	VECTOR3d ret;
	if (mode == UNLOCK_PRO)
	{
		ret.z = a.scale.y * sinf(Deg2Rad(a.rot - 90)) / 2;
		ret.y = -a.scale.y * cosf(Deg2Rad(a.rot - 90)) / 2;
	}
	else if (lpSelect == 1)
	{
		ret.z = a.scale.y * sinf(Deg2Rad(a.rot)) / 2;
		ret.y = -a.scale.y * cosf(Deg2Rad(a.rot)) / 2;
	}
	return ret;
}

VECTOR3d getWidthVector(COLLISON a) { //width vector
	VECTOR3d ret;
	ret.z = a.scale.z * cosf(Deg2Rad(a.rot)) / 2;
	ret.y = -a.scale.z * sinf(Deg2Rad(a.rot)) / 2;
	return ret;
}

VECTOR3d getUnitVector(VECTOR3d a) { //unit vector
	VECTOR3d ret;
	double size;
	size = sqrt(pow(a.z, 2) + pow(a.y, 2));
	ret.z = a.z / size;
	ret.y = a.y / size;
	return ret;
}

bool OBB(COLLISON a, COLLISON b) { //final check
	VECTOR3d dist = getDistanceVector(a, b);
	VECTOR3d vec[4] = { getHeightVector(a), getHeightVector(b), getWidthVector(a), getWidthVector(b) };
	VECTOR3d unit;
	for (int i = 0; i < 4; i++) {
		double sum = 0;
		unit = getUnitVector(vec[i]);
		for (int j = 0; j < 4; j++) {
			sum += absDotVector(vec[j], unit);
		}
		if (absDotVector(dist, unit) > sum) {
			return false;
		}
	}
	return true;
}
