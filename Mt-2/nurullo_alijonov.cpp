#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include "camera.h"
#include "texture.h"
#include "LightSource.h"
#include "Material.h"
#include "Model.h"
#include "stb_image.h"
#include <sstream>
#include <map>
#include <time.h>
#include <vector>

#define WINDOW_WIDTH  1600
#define WINDOW_HEIGHT 900

#define HALF_WIDTH (WINDOW_WIDTH >> 1)     
#define HALF_HEIGHT (WINDOW_HEIGHT >> 1)

// Perspective Camera Parameters
#define FOVY 70.0f
#define NEAR_PLANE 0.01f
#define FAR_PLANE  500.0f

#define  TIMER_PERIOD  16 // Period for the timer.
#define  TIMER_ON     1     // 0:disable timer, 1:enable timer

#define MOUSE_SENSIVITY 4.0f

#define D2R 0.0174532

/* Global Variables for Template File */
bool upKey = false, downKey = false, rightKey = false, leftKey = false;
bool wKey = false, sKey = false, aKey = false, dKey = false, spaceKey = false;
int  winWidth, winHeight; // current Window width and height
bool mode = true;  // F1 key to toggle drawing mode between GL_LINE and GL_FILL
bool capture = true;  // Middle Mouse button set/unset mouse capture
int mouse_x = HALF_WIDTH;
int mouse_y = HALF_HEIGHT;

//
// APPLICATION DATA
//
Model models[13];
Camera cam(0, 5, 20, 0, -10, .5);
Texture flowerTexture;
bool drawMode = false;  /* F1 to toggle between LINE and FILL drawing */

int track[20][17] = {
	{0,0,0,0,7,-1,1,1,8,-1,0,0,0,0,0,0,0},//1
	{0,0,0,0,-1,-1,0,0,-1,-1,0,0,0,0,0,0,0},//2
	{0,0,0,0,2,0,0,0,0,2,0,0,0,0,0,0,0},//3
	{0,0,0,0,2,0,0,0,0,14,0,0,0,0,0,0,0},//4
	{0,0,0,0,2,0,0,0,9,-1,0,0,0,0,0,0,0},//5
	{0,0,0,0,2,0,3,1,-1,-1,0,0,0,0,0,0,0},//6
	{0,0,0,0,2,0,2,0,0,0,0,0,0,0,0,0,0},//7
	{0,0,0,0,2,0,2,0,0,0,0,0,0,0,0,0,0},//8
	{0,0,0,0,13,0,6,1,1,4,0,0,0,0,0,0,0},//9
	{0,0,0,0,11,-1,0,0,12,-1,0,0,0,0,0,0,0},//10
	{0,0,0,0,-1,-1,0,0,-1,-1,0,0,0,0,0,0,0},//11
	{0,0,0,0,0,2,0,0,10,-1,0,0,0,0,0,0,0},//12
	{0,0,0,0,0,2,0,0,-1,-1,1,1,8,-1,0,0,0},//13
	{0,0,0,0,0,14,0,0,0,0,0,0,-1,-1,0,0,0},//14
	{0,0,0,0,0,2,0,0,0,0,0,0,0,2,0,0,0},//15
	{0,0,0,0,0,6,1,4,0,0,0,0,0,2,0,0,0},//16
	{0,0,0,0,0,0,0,10,-1,0,0,0,9,-1,0,0,0},//17
	{0,0,0,0,0,0,0,-1,-1,1,1,1,-1,-1,0,0,0},//18
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},//19
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},//20
}; // track tile locations


int tree[50];
int treePosX[50];
int treePosZ[50];

int flowerPosX[200];
int flowerPosZ[200];
//
// To display onto window using OpenGL commands
//

struct PointLightSource {
	int lightNo;
	float dif_color[4];
	float spec_color[4];
	bool status;
	void init() {
		glLightfv(lightNo, GL_DIFFUSE, dif_color); // Light Source Color
		glLightfv(lightNo, GL_SPECULAR, spec_color);
		if (status) on();
	}
	void addToScene() {
		float origin[] = { 0,0,0,1 }; // 1: point light source, 0 : directional light source
		glLightfv(lightNo, GL_POSITION, origin);
		show();
	}
	void on() {
		glEnable(lightNo);
		status = true;
	}
	void off() {
		glDisable(lightNo);
		status = false;
	}

	void toggle() {
		status = !status;
		if (status) on();
		else off();
	}
	void show() {
		glPushAttrib(GL_ENABLE_BIT); // Save GL_LIGHTING and GL_TEXTURE_2D state 
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);

		glColor3fv(dif_color);
		glutSolidCube(1);  // representative of light source.
		glPopAttrib();  // Restore Saved GL_LIGTHING state.
	}
};


PointLightSource WhiteLS1 = {
	GL_LIGHT0, {1,1,1,1}, {1,1,1,1}, true
};

PointLightSource WhiteLS2 = {
	GL_LIGHT1, {1,1,1,1}, {1,1,1,1}, true
};

void flower() 
{
	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.4);
	glBindTexture(GL_TEXTURE_2D, flowerTexture.id);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);	glVertex3f(-1, -1, 0);
	glTexCoord2f(1, 0);	glVertex3f(1, -1, 0);
	glTexCoord2f(1, 1); glVertex3f(1, 1, 0);
	glTexCoord2f(0, 1); glVertex3f(-1, 1, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, flowerTexture.id);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);	glVertex3f(0, -1, -1);
	glTexCoord2f(1, 0);	glVertex3f(0, -1, 1);
	glTexCoord2f(1, 1); glVertex3f(0, 1, 1);
	glTexCoord2f(0, 1); glVertex3f(0, 1, -1);
	glEnd();

	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
}

void drawTrack() // draws track
{
	int treeIter = 0;

	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 17; j++) {
			switch (track[i][j])
			{
			case 0:
			{
				glPushMatrix();
				glTranslatef(j * 8, -2, i * 8);
				models[0].render();
				glPopMatrix();

				//Drawing Trees on Grass Tiles

				//if (tree[i][j] != 0) {
				//	glPushMatrix();
				//	glTranslatef(treePosX[treeIter] + (8 * j), 3, treePosZ[treeIter] + (8 * i));
				//	glScalef(0.025, 0.025, 0.025);
				//	models[tree[i][j]].render();
				//	glPopMatrix();
				//	treeIter++;
				//}
				
				break;
			}
			case 1:
			{
				glPushMatrix();
				glTranslatef(j * 8, -2, i * 8);
				glRotatef(90, 0, 1, 0);
				models[5].render();
				glPopMatrix();
				break;
			}
			case 2:
			{
				glPushMatrix();
				glTranslatef(j * 8, -2, i * 8);
				models[5].render();
				glPopMatrix();
				break;
			}
			case 3:
			{
				glPushMatrix();
				glTranslatef(j * 8, -2, i * 8);
				glRotatef(90, 0, 1, 0);
				models[2].render();
				glPopMatrix();
				break;
			}
			case 4:
			{
				glPushMatrix();
				glTranslatef(j * 8, -2, i * 8);
				models[2].render();
				glPopMatrix();
				break;
			}
			case 5:
			{
				glPushMatrix();
				glTranslatef(j * 8, -2, i * 8);
				glRotatef(-90, 0, 1, 0);
				models[2].render();
				glPopMatrix();
				break;
			}
			case 6:
			{
				glPushMatrix();
				glTranslatef(j * 8, -2, i * 8);
				glRotatef(-180, 0, 1, 0);
				models[2].render();
				glPopMatrix();
				break;
			}
			case 7:
			{
				glPushMatrix();
				glTranslatef(j * 8 + 4, -2, i * 8 + 4);
				glRotatef(90, 0, 1, 0);
				models[1].render();
				glPopMatrix();
				break;
			}
			case 8:
			{
				glPushMatrix();
				glTranslatef(j * 8 + 4, -2, i * 8 + 4);
				models[1].render();
				glPopMatrix();
				break;
			}
			case 9:
			{
				glPushMatrix();
				glTranslatef(j * 8 + 4, -2, i * 8 + 4);
				glRotatef(-90, 0, 1, 0);
				models[1].render();
				glPopMatrix();
				break;
			}
			case 10:
			{
				glPushMatrix();
				glTranslatef(j * 8 + 4, -2, i * 8 + 4);
				glRotatef(-180, 0, 1, 0);
				models[1].render();
				glPopMatrix();
				break;
			}
			case 11:
			{
				glPushMatrix();
				glTranslatef(j * 8 + 4, -2, i * 8 + 4);
				glRotatef(90, 0, 1, 0);
				models[4].render();
				glPopMatrix();
				break;
			}
			case 12:
			{
				glPushMatrix();
				glTranslatef(j * 8 + 4, -2, i * 8 + 4);
				glRotatef(-90, 0, 1, 0);
				models[3].render();
				glPopMatrix();
				break;
			}
			case 13: // race track start
			{
				glPushMatrix();
				glTranslatef(j * 8, -2, i * 8);
				glRotatef(90, 0, 1, 0);
				models[6].render();
				glPopMatrix();

				// placing a car at the start of the race track
				glPushMatrix();
				glTranslatef(j * 8, -1, i * 8);
				glScalef(1.2, 1.2, 1.2);
				glRotatef(180, 0, 1, 0);
				models[12].render();
				glPopMatrix();

				break;
			}
			case 14:
			{
				glPushMatrix();
				glTranslatef(j * 8, -2, i * 8);
				glRotatef(90, 0, 1, 0);
				models[7].render();
				glPopMatrix();
				break;
			}
			case -1:
			{
				break;
			}
			}
		}
	}
	

}

void display()
{  
    glClearColor(0.5, 0.6, 0.85, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	cam.LookAt();

	glPushMatrix();
	glTranslatef(4 * 8, 60, 8 * 16);
	WhiteLS1.addToScene();
	WhiteLS1.show();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(16 * 8, 60, 8 * 4);
	WhiteLS2.addToScene();
	WhiteLS2.show();
	glPopMatrix();


	//Tiles, each tile dimension is 4x2x4 (x,y,z)
	//for (int i = 0; i < 6; i++) {
	//	glPushMatrix();
	//		glTranslatef(i * 20 - 60, -2, -10);
	//		models[i].render();
	//	glPopMatrix();
	//}

	//glColor4f(1.0, 1.0, 1.0, 1);
	for (int i = 0; i < 200; i++) {
		glPushMatrix();
		glTranslatef(flowerPosX[i] - 4, 0, flowerPosZ[i] - 4);
		flower();
		glPopMatrix();
	}
	
	for (int i = 0; i < 50; i++) {
		glPushMatrix();
		glTranslatef(treePosX[i] - 4, 3, treePosZ[i] - 4);
		glScalef(0.025, 0.025, 0.025);
		models[tree[i]].render();
		glPopMatrix();
	}


	drawTrack();

	glutSwapBuffers();
}

//
// key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
//


void onKeydown(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);

	switch (key) {
	case 'w':
	case 'W': wKey = true; break;
	case 's':
	case 'S': sKey = true; break;
	case 'a':
	case 'A': aKey = true; break;
	case 'd':
	case 'D': dKey = true; break;
	case ' ': spaceKey = true; break;
	}
}

void onKeyup(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);

	switch (key) {
	case 'w':
	case 'W': wKey = false; break;
	case 's':
	case 'S': sKey = false; break;
	case 'a':
	case 'A': aKey = false; break;
	case 'd':
	case 'D': dKey = false; break;
	case ' ': spaceKey = false; break;
	}
}

void onSpecialKeydown(int key, int x, int y)
{
	// Write your codes here.
	switch (key) {
	case GLUT_KEY_UP: upKey = true;	break;
	case GLUT_KEY_DOWN: downKey = true; break;
	case GLUT_KEY_LEFT: leftKey = true; break;
	case GLUT_KEY_RIGHT: rightKey = true; break;	
	}
}

void onSpecialKeyup(int key, int x, int y)
{
	// Write your codes here.
	switch (key) {
	case GLUT_KEY_UP: upKey = false;  break;
	case GLUT_KEY_DOWN: downKey = false; break;
	case GLUT_KEY_LEFT: leftKey = false; break;
	case GLUT_KEY_RIGHT: rightKey = false; break;
	case GLUT_KEY_F1: drawMode = !drawMode; 
		if (drawMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	}
}


void onClick(int button, int status, int x, int y)
{
	// Write your codes here.
	if (button == GLUT_MIDDLE_BUTTON && status == GLUT_DOWN) {
		capture = !capture;
		if (capture) {
			glutSetCursor(GLUT_CURSOR_NONE);
			glutWarpPointer(HALF_WIDTH, HALF_HEIGHT);
			mouse_x = HALF_WIDTH;
			mouse_y = HALF_HEIGHT;
		}
		else {
			glutSetCursor(GLUT_CURSOR_INHERIT);
		}
	}
	
}

void onResize(int w, int h)
{
	winWidth = w;
	winHeight = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(FOVY, winWidth * 1.0f / winHeight, NEAR_PLANE, FAR_PLANE);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0, 0, 0, 0);
	display(); // refresh window.
}

void onMove(int x, int y) {
	// Write your codes here.
	mouse_x = x;
	mouse_y = y;
}

void onTimer(int v) {
	
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
	// Write your codes here.

	
	if (wKey) {
		cam.Forward();
	}
	if (sKey) {
		cam.Backward();
	}

	if (aKey) {
		cam.Left();
	}

	if (dKey) {
		cam.Right();
	}

			
	if (capture) {
		cam.TurnRightLeft((HALF_WIDTH - mouse_x) / MOUSE_SENSIVITY);
		cam.TurnUpDown((HALF_HEIGHT - mouse_y) / MOUSE_SENSIVITY);
		glutWarpPointer( HALF_WIDTH, HALF_HEIGHT);
	}
	
	glutPostRedisplay(); // display()

}

bool checkIfGrass(int posX, int posZ) {
	int x = posX / 8;
	int z = posZ / 8;
	if (track[z][x] != 0) {
		return false;
	}
	return true;
}

void randomizeFlowers() {
	srand(time(NULL));
	int flPosX = 0;
	int flPosZ = 0;
	int numberOfFlowers = 0;
	while(numberOfFlowers != 200) {
		flPosX = rand() % 137;
		flPosZ = rand() % 161;
		if (checkIfGrass(flPosX, flPosZ)) {
			flowerPosX[numberOfFlowers] = flPosX;
			flowerPosZ[numberOfFlowers] = flPosZ;
			numberOfFlowers++;
		}
	}
}

void randomizeTrees() {
	srand(time(NULL));
	int treeType = 0;
	int trPosX = 0;
	int trPosZ = 0;
	int numberOfTrees = 0;
	while (numberOfTrees != 50) {
		trPosX = rand() % 137;
		trPosZ = rand() % 161;
		treeType = rand() % 4 + 8;
		if (checkIfGrass(trPosX, trPosZ)) {
			treePosX[numberOfTrees] = trPosX;
			treePosZ[numberOfTrees] = trPosZ;
			tree[numberOfTrees] = treeType;
			numberOfTrees++;
		}
	}
}

void Init() {
	
	// Load Models
	models[0].init("models/racetrack", "terrain-grass.obj");
	models[1].init("models/racetrack", "track-corner-large.obj");
	models[2].init("models/racetrack", "track-corner-small.obj");
	models[3].init("models/racetrack", "track-left-right-large.obj");
	models[4].init("models/racetrack", "track-right-left-large.obj");
	models[5].init("models/racetrack", "track-straight-small.obj");
	models[6].init("models/racetrack", "track-start-finish.obj");
	models[7].init("models/racetrack", "track-tire.obj");
	models[8].init("models/tree/tree1", "tree1.obj");
	models[9].init("models/tree/tree2", "tree2.obj");
	models[10].init("models/tree/tree3", "tree3.obj");
	models[11].init("models/tree/tree4", "tree4.obj");
	models[12].init("models/car", "car-formula.obj");
	
	flowerTexture.Load("texture/flower1.png");

	// Enable lighting and texturing
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_NORMALIZE); // to normalize Normal vector for uniform scaling.
	//glEnable(GL_LIGHTING);
	
	WhiteLS1.init();

	WhiteLS2.init();


	glEnable(GL_LIGHTING);

	// set the cursor position
	glutWarpPointer(HALF_WIDTH, HALF_HEIGHT);
	// hide cursor
	glutSetCursor(GLUT_CURSOR_NONE);
	
	randomizeTrees(); // randomize tree pos
	randomizeFlowers(); // randomize flowers
	glEnable(GL_DEPTH_TEST);	
}

void main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("CTIS489 - Midterm #2");
	
	Init();

	glutDisplayFunc(display);
	glutReshapeFunc(onResize);
	//
	// keyboard registration
	//
	glutKeyboardFunc(onKeydown);
	glutSpecialFunc(onSpecialKeydown);

	glutKeyboardUpFunc(onKeyup);
	glutSpecialUpFunc(onSpecialKeyup);

	//
	// mouse registration
	//
	glutMouseFunc(onClick);
	glutPassiveMotionFunc(onMove);

	// timer event
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
	glutMainLoop();
}