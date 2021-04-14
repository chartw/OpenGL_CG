#ifndef FINAL__H
#define FINAL__H

#include "bmpfuncs.h"
#include "ObjParser.h"
#include "mmsystem.h"
#pragma comment(lib,"winmm.lib")

#include <gl/glut.h>
#include <gl/freeglut.h>
#include <gl/glext.h>
#include <stdio.h>
#include <cmath>
#include <vector>
#include <utility>
#include <Windows.h>
using namespace std;

#define CUBE 0
#define CYLINDER 1
#define SPHERE 2


#define PART_VIEWER 1
#define UNLOCK_PRO 2
#define ESCAPE 3

#define SILVER 1
#define GOLD 2
#define DSILVER 3
#define CSILVER 4
#define CGOLD 5
#define WH 6
#define ROOM 7

#define PLUG 0
#define CY 1
#define OCAM 2
#define KEY1 3
#define ICAM 4
#define KEY2 16
#define WASH 15
#define KPIN 6
#define DPIN 11

struct VECTOR3d
{
	double x;
	double y;
	double z;
};

struct COLLISON
{
	VECTOR3d position;
	VECTOR3d scale;
	double rot;
};

double absDotVector(VECTOR3d a, VECTOR3d b);
double Deg2Rad(double deg);
VECTOR3d getDistanceVector(COLLISON a, COLLISON b);
VECTOR3d getHeightVector(COLLISON a);
VECTOR3d getWidthVector(COLLISON a);
VECTOR3d getUnitVector(VECTOR3d a);
bool OBB(COLLISON a, COLLISON b);

void key_pin_OBB();
void lp_pin_OBB();

void camInit();
void lightInit();
void unlockPosInit();
void pickingInit();

void draw_axis(void);
void draw_string(void* font, const char* str, float x, float y, float red, float green, float blue);
void draw_text(const char* str, int r, int g, int b);

void draw_item_box();
void draw_item();
void draw_lockpick();
void draw_lockpicking();
void draw_room_obj();
void draw_full();
void draw_half();
void draw_key();
void draw_kpin();
void draw_dpin();
void draw_incam();

void draw_obj(ObjParser* objParser);
void draw_obj_tx(ObjParser* objParser, GLuint texture);

void setTextureMapping(GLuint texture);
void skyTextureMapping(int skytex);
void draw_textureSkybox(GLuint g_nCubeTex);

void ItemPicking(int x, int y);
void Picking(int x, int y);
void DrawPartView();

void ass_disass();
void print_man();
void move_lightpos(GLfloat* pos, GLfloat x, GLfloat y, GLfloat z);

#endif