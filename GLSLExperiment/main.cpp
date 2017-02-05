/*
 * main.cpp
 * John Nelson - jpnelson@wpi.edu
 * Homework 2
 */

#include "Angel.h"
#include "Polygon.h"
#include "FileReader.h"
#include <algorithm>

// remember to prototype
void readMeshes(void);
void scaleMeshes(void);
void createHierarchy(void);
void generateGeometry(void);
void swapShaderPrograms(GLuint p);
void display(void);
void drawMesh(int index);
void drawAxis(void);
void drawArm(int index);
void passToShader(float view[16], float model[16]);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void reshape(GLsizei w, GLsizei h);
void idle(void);
point4 calculateCenterPoint(point4 a, point4 b, point4 c);

typedef Angel::vec4 color4;
typedef Angel::vec4 point4;

//---------------------------- Variables ----------------------------------------

enum shader_type {
	shader_flat, shader_smooth
};

// handle to program
GLuint programFlat, programSmooth, program;

using namespace std;

const int NUM_PLY_FILES = 7;
const int NUM_AXIS_POINTS = 6;
const int NUM_ARM_POINTS = 24;
const int NUM_BOX_POINTS = 168;
const int ARM_POINT_SIZE = 8;
const int BOX_POINT_SIZE = 24;

const int FRAME_RATE = 30;	//30 frames per second

const float MODEL_SIZE = 2.0f;
const float MODEL_THETA = 1.0f;
const float MODEL_OFFSET = 2.0f;

int width = 0;
int height = 0;

int frames = 0;
int curMesh = 0;

int numVertices;

float sinusoid = 90.0f;
float spotLightAngle = 0.0f;

bool isShowingExtents = false;
bool isFollowingSinusoid = false;
bool isUsingFlatShading = true;
bool isShowingAxis = false;
bool isRotating = true;

GLfloat camera_fovy = 45.0f;
GLfloat camera_near = 0.1f;
GLfloat camera_far = 100.0f;

MyMesh *mesh[NUM_PLY_FILES];
MyMesh arm_mesh[3];
MyMesh empty_mesh[NUM_PLY_FILES];

point4 *normals;
point4 *points;
color4 *colors;

vec4 lightPos = vec4(-10.0, 10.0, 10.0, 1.0);
vec4 spotLightPos = vec4(0, 0, 10, 1);
vec4 spotLightDirection = vec4(0, 0, -1, 1);

#pragma region Vert_Col_Quad_ColCube

// RGBA olors
color4 vertex_colors[8] = {
	color4(1.0, 1.0, 1.0, 1.0),	 // white
	color4(1.0, 0.0, 0.0, 1.0),  // red
	color4(1.0, 1.0, 0.0, 1.0),  // yellow
	color4(0.0, 1.0, 0.0, 1.0),  // green
	color4(0.0, 0.0, 1.0, 1.0),  // blue
	color4(1.0, 0.0, 1.0, 1.0),  // magenta
	color4(0.0, 1.0, 1.0, 1.0),  // cyan
	color4(0.0, 0.0, 0.0, 1.0)  // black
};

#pragma endregion

//----------------------------------------------------------------------------
// entry point
int main(int argc, char **argv)
{
	// init glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	width = 512;
	height = 512;

	// create window
	// opengl can be incorperated into other packages like wxwidgets, fltoolkit, etc.
	glutCreateWindow("Color Cube");

	// init glew
	glewInit();

	readMeshes();
	scaleMeshes();
	createHierarchy();

	generateGeometry();

	// assign handlers
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);

	// add mouse handler
	glutMouseFunc(mouse);

	// add resize window functionality (should probably try to preserve aspect ratio)
	glutReshapeFunc(reshape);

	// idle
	glutIdleFunc(idle);

	// enter the drawing loop
	// frame rate can be controlled with 
	glutMainLoop();

	return 0;
}

void readMeshes(void) {
	mesh[0] = readPLYFile("./ply_files/cow.ply");
	mesh[1] = readPLYFile("./ply_files/ant.ply");
	mesh[2] = readPLYFile("./ply_files/teapot.ply");
	mesh[3] = readPLYFile("./ply_files/shark.ply");
	mesh[4] = readPLYFile("./ply_files/beethoven.ply");
	mesh[5] = readPLYFile("./ply_files/galleon.ply");
	mesh[6] = readPLYFile("./ply_files/big_dodge.ply");

	for (int i = 0; i < 3; i++) {
		point4 *pos = new point4[8];

		//Top Line
		pos[0] = point4(0, 0, 0, 1);
		pos[1] = point4(0, -(MODEL_OFFSET / 2), 0, 1);

		//Crossbar Line
		pos[2] = point4(-MODEL_OFFSET, -(MODEL_OFFSET / 2), 0, 1);
		pos[3] = point4(MODEL_OFFSET, -(MODEL_OFFSET / 2), 0, 1);

		//Arm Line 1
		pos[4] = point4(-MODEL_OFFSET, -(MODEL_OFFSET / 2), 0, 1);
		pos[5] = point4(-MODEL_OFFSET, -MODEL_OFFSET, 0, 1);

		//Arm Line 2
		pos[6] = point4(MODEL_OFFSET, -(MODEL_OFFSET / 2), 0, 1);
		pos[7] = point4(MODEL_OFFSET, -MODEL_OFFSET, 0, 1);

		arm_mesh[i] = MyMesh(pos, 8, NULL, 0, false);
	}

	for (int i = 0; i < NUM_PLY_FILES; i++) {
		empty_mesh[i] = MyMesh(NULL, 0, NULL, 0, false);
	}
}

void scaleMeshes(void) {
	for (int i = 0; i < NUM_PLY_FILES; i++) {
		mesh[i]->setScale(mesh[i]->getScaleToDimensions(MODEL_SIZE));
		mesh[i]->setPositionToOrigin();
		cout << "Mesh " << i << ": " << mesh[i]->getScale() << "\n";
	}

}

void createHierarchy(void) {
	//Level 3 Meshes
	mesh[5]->setParent(&empty_mesh[5]);
	mesh[6]->setParent(&empty_mesh[6]);
	empty_mesh[5].setParent(&arm_mesh[2]);
	empty_mesh[6].setParent(&arm_mesh[2]);
	arm_mesh[2].setParent(&empty_mesh[4]);

	//Level 2 Meshes
	mesh[3]->setParent(&empty_mesh[3]);
	mesh[4]->setParent(&empty_mesh[4]);
	empty_mesh[3].setParent(&arm_mesh[1]);
	empty_mesh[4].setParent(&arm_mesh[1]);
	arm_mesh[1].setParent(&empty_mesh[2]);

	//Level 1 Meshes
	mesh[1]->setParent(&empty_mesh[1]);
	mesh[2]->setParent(&empty_mesh[2]);
	empty_mesh[1].setParent(&arm_mesh[0]);
	empty_mesh[2].setParent(&arm_mesh[0]);
	arm_mesh[0].setParent(&empty_mesh[0]);

	//Level 0 Meshes
	mesh[0]->setParent(&empty_mesh[0]);

	//Set positions
	empty_mesh[0].setPosition(vec3(0.0f, MODEL_OFFSET * 1.5, -MODEL_OFFSET));

	arm_mesh[0].setLocalPosition(vec3(0));
	empty_mesh[1].setLocalPosition(vec3(-MODEL_OFFSET, -MODEL_OFFSET, 0.0f));
	empty_mesh[2].setLocalPosition(vec3(MODEL_OFFSET, -MODEL_OFFSET, 0.0f));

	arm_mesh[1].setLocalPosition(vec3(0));
	empty_mesh[3].setLocalPosition(vec3(-MODEL_OFFSET, -MODEL_OFFSET, 0.0f));
	empty_mesh[4].setLocalPosition(vec3(MODEL_OFFSET, -MODEL_OFFSET, 0.0f));

	arm_mesh[2].setLocalPosition(vec3(0));
	empty_mesh[5].setLocalPosition(vec3(-MODEL_OFFSET, -MODEL_OFFSET, 0.0f));
	empty_mesh[6].setLocalPosition(vec3(MODEL_OFFSET, -MODEL_OFFSET, 0.0f));

	//Rotate Objects that need it
	mesh[2]->setLocalRotation(270, 0, 0);
	mesh[5]->setLocalRotation(270, 0, 0);
}

void generateGeometry(void)
{
#pragma region FILL_GEOMETRY
	//Determine number of points, colors, and normals
	numVertices = 0 + NUM_AXIS_POINTS + NUM_ARM_POINTS + NUM_BOX_POINTS;
	int numFaces = 0;
	int iter = 0;

	for (int i = 0; i < NUM_PLY_FILES; i++) {
		if (mesh[i] != NULL) {
			mesh[i]->setStartingIndex(numVertices);
			numVertices += mesh[i]->getTotalVertices();
		}
	}

	points = new point4[numVertices];
	colors = new point4[numVertices];
	normals = new point4[numVertices];

	//Generate array of vertices, colors, and normals for models

	//Axis points
	points[0] = point4(-100.0f, 0.0f, 0.0f, 1.0f);
	points[1] = point4(100.0f, 0.0f, 0.0f, 1.0f);
	points[2] = point4(0.0f, 100.0f, 0.0f, 1.0f);
	points[3] = point4(0.0f, -100.0f, 0.0f, 1.0f);
	points[4] = point4(0.0f, 0.0f, 100.0f, 1.0f);
	points[5] = point4(0.0f, 0.0f, -100.0f, 1.0f);

	for (int i = 0; i < NUM_AXIS_POINTS; i++) {
		colors[i] = vertex_colors[1];
		normals[i] = lightPos - points[i];
	}

	//Arm Points
	for (int i = 0; i < 3; i++) {
		int x = NUM_AXIS_POINTS + i * ARM_POINT_SIZE;

		for (int j = 0; j < 8; j++) {
			points[x + j] = arm_mesh[i].getVertices()[j];
		}
	}

	for (int i = 0; i < NUM_ARM_POINTS; i++) {
		colors[NUM_AXIS_POINTS + i] = vertex_colors[7];
		normals[NUM_AXIS_POINTS + i] = lightPos;
	}

	//Extent Box Points
	for (int i = 0; i < NUM_PLY_FILES; i++) {	//for each mesh
		if (mesh[i] != NULL) {
			int x = NUM_AXIS_POINTS + NUM_ARM_POINTS + i * BOX_POINT_SIZE;
			vec3 l = mesh[i]->getLowerBounds();
			vec3 u = mesh[i]->getUpperBounds();

			/*** Bottom Lines ***/

			//Line 1
			points[x] = point4(l.x, l.y, l.z, 1.0f);
			points[x + 1] = point4(u.x, l.y, l.z, 1.0f);

			//Line 2
			points[x + 2] = point4(l.x, l.y, l.z, 1.0f);
			points[x + 3] = point4(l.x, l.y, u.z, 1.0f);

			//Line 3
			points[x + 4] = point4(u.x, l.y, l.z, 1.0f);
			points[x + 5] = point4(u.x, l.y, u.z, 1.0f);

			//Line 4
			points[x + 6] = point4(l.x, l.y, u.z, 1.0f);
			points[x + 7] = point4(u.x, l.y, u.z, 1.0f);

			/*** Vertical Lines ***/

			//Line 5
			points[x + 8] = point4(l.x, l.y, l.z, 1.0f);
			points[x + 9] = point4(l.x, u.y, l.z, 1.0f);

			//Line 6
			points[x + 10] = point4(u.x, l.y, l.z, 1.0f);
			points[x + 11] = point4(u.x, u.y, l.z, 1.0f);

			//Line 7
			points[x + 12] = point4(l.x, l.y, u.z, 1.0f);
			points[x + 13] = point4(l.x, u.y, u.z, 1.0f);

			//Line 8
			points[x + 14] = point4(u.x, l.y, u.z, 1.0f);
			points[x + 15] = point4(u.x, u.y, u.z, 1.0f);

			/*** Top Lines ***/

			//Line 9
			points[x + 16] = point4(l.x, u.y, l.z, 1.0f);
			points[x + 17] = point4(u.x, u.y, l.z, 1.0f);

			//Line 10
			points[x + 18] = point4(l.x, u.y, l.z, 1.0f);
			points[x + 19] = point4(l.x, u.y, u.z, 1.0f);

			//Line 11
			points[x + 20] = point4(u.x, u.y, l.z, 1.0f);
			points[x + 21] = point4(u.x, u.y, u.z, 1.0f);

			//Line 12
			points[x + 22] = point4(l.x, u.y, u.z, 1.0f);
			points[x + 23] = point4(u.x, u.y, u.z, 1.0f);
		}
	}

	iter = NUM_AXIS_POINTS + NUM_ARM_POINTS;

	for (int i = 0; i < NUM_BOX_POINTS; i++) {
		colors[iter + i] = vertex_colors[(int)(i / BOX_POINT_SIZE)];
		normals[iter + i] = lightPos;
	}

	iter = NUM_AXIS_POINTS + NUM_ARM_POINTS + NUM_BOX_POINTS;

	//Mesh points
	for (int i = 0; i < NUM_PLY_FILES; i++) {	//for each mesh
		if (mesh[i] != NULL) {
			for (int j = 0; j < mesh[i]->getNumFaces(); j++) {	//for each mesh face
				//Positions
				points[iter + j * 3] = mesh[i]->getFaceX(j);
				points[iter + j * 3 + 1] = mesh[i]->getFaceY(j);
				points[iter + j * 3 + 2] = mesh[i]->getFaceZ(j);

				//Colors
				colors[iter + j * 3] = vertex_colors[i];
				colors[iter + j * 3 + 1] = vertex_colors[i];
				colors[iter + j * 3 + 2] = vertex_colors[i];

				//Normals
				normals[iter + j * 3] = mesh[i]->getNormals()[(int)mesh[i]->getFaces()[j].x];
				normals[iter + j * 3 + 1] = mesh[i]->getNormals()[(int)mesh[i]->getFaces()[j].y];
				normals[iter + j * 3 + 2] = mesh[i]->getNormals()[(int)mesh[i]->getFaces()[j].z];
			}
			iter += mesh[i]->getTotalVertices();
		}
	}

#pragma endregion

	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(point4) * numVertices)
								+ (sizeof(point4) * numVertices)
								+ (sizeof(point4) * numVertices), NULL, GL_STATIC_DRAW);

	//Add points and colors to buffer data
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * numVertices, points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * numVertices, sizeof(point4) * numVertices, colors);
	glBufferSubData(GL_ARRAY_BUFFER, (sizeof(point4) * numVertices) + (sizeof(point4) * numVertices), 
					sizeof(point4) * numVertices, normals);

	// Load shaders and use the resulting shader program
	programFlat = InitShader("vshader1.glsl", "fshader1.glsl");
	programSmooth = InitShader("vshader2.glsl", "fshader2.glsl");
	
	if (isUsingFlatShading)
		swapShaderPrograms(programFlat);
	else
		swapShaderPrograms(programSmooth);
}

void swapShaderPrograms(GLuint p) {
	program = p;
	glUseProgram(program);

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4) * numVertices));

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4) * numVertices * 2));

	// sets the default color to clear screen
	glClearColor(1.0, 1.0, 1.0, 1.0); // white background
}

//----------------------------------------------------------------------------
// this is where the drawing should happen
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // clear the window

	if(isShowingAxis)
		drawAxis();

	for (int i = 0; i < NUM_PLY_FILES; i++) {
		drawMesh(i);
	}

	for (int i = 0; i < 3; i++) {
		drawArm(i);
	}

	glFlush(); // force output to graphics hardware

	// use this call to double buffer
	glutSwapBuffers();

	// you can implement your own buffers with textures
}

void drawMesh(int index) {
	/***	projection matrix	***/

	GLfloat aspectRatio = (GLfloat)width / (GLfloat)height;

	GLfloat cameraZOffset = 10.0f;

	Angel::mat4 perspectiveMat = Angel::Perspective(camera_fovy, aspectRatio, cameraZOffset / 2, cameraZOffset * 2);

	float viewMatrixf[16];
	viewMatrixf[0] = perspectiveMat[0][0]; viewMatrixf[4] = perspectiveMat[0][1];
	viewMatrixf[1] = perspectiveMat[1][0]; viewMatrixf[5] = perspectiveMat[1][1];
	viewMatrixf[2] = perspectiveMat[2][0]; viewMatrixf[6] = perspectiveMat[2][1];
	viewMatrixf[3] = perspectiveMat[3][0]; viewMatrixf[7] = perspectiveMat[3][1];

	viewMatrixf[8] = perspectiveMat[0][2]; viewMatrixf[12] = perspectiveMat[0][3];
	viewMatrixf[9] = perspectiveMat[1][2]; viewMatrixf[13] = perspectiveMat[1][3];
	viewMatrixf[10] = perspectiveMat[2][2]; viewMatrixf[14] = perspectiveMat[2][3];
	viewMatrixf[11] = perspectiveMat[3][2]; viewMatrixf[15] = perspectiveMat[3][3];

	/***	Model Matrix	***/

	Angel::mat4 model = Angel::identity();

	mat4 view = Angel::LookAt(vec4(0, 0, cameraZOffset, 1),
		vec4(0, 0, 0, 1),
		vec4(0, 1, 0, 1));

	mat4 modelMat = view * mesh[index]->getCTM() * model;

	float modelMatrixf[16];

	modelMatrixf[0] = modelMat[0][0]; modelMatrixf[4] = modelMat[0][1];
	modelMatrixf[1] = modelMat[1][0]; modelMatrixf[5] = modelMat[1][1];
	modelMatrixf[2] = modelMat[2][0]; modelMatrixf[6] = modelMat[2][1];
	modelMatrixf[3] = modelMat[3][0]; modelMatrixf[7] = modelMat[3][1];

	modelMatrixf[8] = modelMat[0][2]; modelMatrixf[12] = modelMat[0][3];
	modelMatrixf[9] = modelMat[1][2]; modelMatrixf[13] = modelMat[1][3];
	modelMatrixf[10] = modelMat[2][2]; modelMatrixf[14] = modelMat[2][3];
	modelMatrixf[11] = modelMat[3][2]; modelMatrixf[15] = modelMat[3][3];

	// set up projection matricies
	passToShader(viewMatrixf, modelMatrixf);

	// change to GL_FILL
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// draw functions should enable then disable the features 
	// that are specifit the themselves
	// the depth is disabled after the draw 
	// in case you need to draw overlays
	glEnable(GL_DEPTH_TEST);

	glDrawArrays(GL_TRIANGLES, mesh[index]->getStartingIndex(), mesh[index]->getTotalVertices());

	if (isShowingExtents) {
		glDrawArrays(GL_LINES, NUM_AXIS_POINTS + NUM_ARM_POINTS + index * BOX_POINT_SIZE, BOX_POINT_SIZE);
	}

	glDisable(GL_DEPTH_TEST);
}

void drawAxis(void) {
	GLfloat aspectRatio = (GLfloat)width / (GLfloat)height;

	GLfloat cameraZOffset = 10.0f;

	Angel::mat4 perspectiveMat = Angel::Perspective(camera_fovy, aspectRatio, cameraZOffset / 2, cameraZOffset * 2);

	float viewMatrixf[16];
	viewMatrixf[0] = perspectiveMat[0][0]; viewMatrixf[4] = perspectiveMat[0][1];
	viewMatrixf[1] = perspectiveMat[1][0]; viewMatrixf[5] = perspectiveMat[1][1];
	viewMatrixf[2] = perspectiveMat[2][0]; viewMatrixf[6] = perspectiveMat[2][1];
	viewMatrixf[3] = perspectiveMat[3][0]; viewMatrixf[7] = perspectiveMat[3][1];

	viewMatrixf[8] = perspectiveMat[0][2]; viewMatrixf[12] = perspectiveMat[0][3];
	viewMatrixf[9] = perspectiveMat[1][2]; viewMatrixf[13] = perspectiveMat[1][3];
	viewMatrixf[10] = perspectiveMat[2][2]; viewMatrixf[14] = perspectiveMat[2][3];
	viewMatrixf[11] = perspectiveMat[3][2]; viewMatrixf[15] = perspectiveMat[3][3];

	/***	Model Matrix	***/

	Angel::mat4 modelMat = Angel::identity();

	//Look at
	GLfloat cameraZ = cameraZOffset;
	modelMat = modelMat * Angel::LookAt(vec4(0, 0, cameraZ, 1),
		vec4(0, 0, 0, 1),
		vec4(0, 1, 0, 1));

	float modelMatrixf[16];

	modelMatrixf[0] = modelMat[0][0]; modelMatrixf[4] = modelMat[0][1];
	modelMatrixf[1] = modelMat[1][0]; modelMatrixf[5] = modelMat[1][1];
	modelMatrixf[2] = modelMat[2][0]; modelMatrixf[6] = modelMat[2][1];
	modelMatrixf[3] = modelMat[3][0]; modelMatrixf[7] = modelMat[3][1];

	modelMatrixf[8] = modelMat[0][2]; modelMatrixf[12] = modelMat[0][3];
	modelMatrixf[9] = modelMat[1][2]; modelMatrixf[13] = modelMat[1][3];
	modelMatrixf[10] = modelMat[2][2]; modelMatrixf[14] = modelMat[2][3];
	modelMatrixf[11] = modelMat[3][2]; modelMatrixf[15] = modelMat[3][3];

	// set up projection matricies
	passToShader(viewMatrixf, modelMatrixf);

	// change to GL_FILL
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// draw functions should enable then disable the features 
	// that are specifit the themselves
	// the depth is disabled after the draw 
	// in case you need to draw overlays
	glEnable(GL_DEPTH_TEST);

	glDrawArrays(GL_LINES, 0, NUM_AXIS_POINTS);

	glDisable(GL_DEPTH_TEST);
}

void drawArm(int index) {
	GLfloat aspectRatio = (GLfloat)width / (GLfloat)height;

	GLfloat cameraZOffset = 10.0f;

	Angel::mat4 perspectiveMat = Angel::Perspective(camera_fovy, aspectRatio, cameraZOffset / 2, cameraZOffset * 2);

	float viewMatrixf[16];
	viewMatrixf[0] = perspectiveMat[0][0]; viewMatrixf[4] = perspectiveMat[0][1];
	viewMatrixf[1] = perspectiveMat[1][0]; viewMatrixf[5] = perspectiveMat[1][1];
	viewMatrixf[2] = perspectiveMat[2][0]; viewMatrixf[6] = perspectiveMat[2][1];
	viewMatrixf[3] = perspectiveMat[3][0]; viewMatrixf[7] = perspectiveMat[3][1];

	viewMatrixf[8] = perspectiveMat[0][2]; viewMatrixf[12] = perspectiveMat[0][3];
	viewMatrixf[9] = perspectiveMat[1][2]; viewMatrixf[13] = perspectiveMat[1][3];
	viewMatrixf[10] = perspectiveMat[2][2]; viewMatrixf[14] = perspectiveMat[2][3];
	viewMatrixf[11] = perspectiveMat[3][2]; viewMatrixf[15] = perspectiveMat[3][3];

	/***	Model Matrix	***/

	Angel::mat4 model = Angel::identity();

	mat4 view = Angel::LookAt(vec4(0, 0, cameraZOffset, 1),
		vec4(0, 0, 0, 1),
		vec4(0, 1, 0, 1));

	mat4 modelMat = view * arm_mesh[index].getCTM() * model;

	float modelMatrixf[16];

	modelMatrixf[0] = modelMat[0][0]; modelMatrixf[4] = modelMat[0][1];
	modelMatrixf[1] = modelMat[1][0]; modelMatrixf[5] = modelMat[1][1];
	modelMatrixf[2] = modelMat[2][0]; modelMatrixf[6] = modelMat[2][1];
	modelMatrixf[3] = modelMat[3][0]; modelMatrixf[7] = modelMat[3][1];

	modelMatrixf[8] = modelMat[0][2]; modelMatrixf[12] = modelMat[0][3];
	modelMatrixf[9] = modelMat[1][2]; modelMatrixf[13] = modelMat[1][3];
	modelMatrixf[10] = modelMat[2][2]; modelMatrixf[14] = modelMat[2][3];
	modelMatrixf[11] = modelMat[3][2]; modelMatrixf[15] = modelMat[3][3];

	// set up projection matricies
	passToShader(viewMatrixf, modelMatrixf);

	// change to GL_FILL
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// draw functions should enable then disable the features 
	// that are specifit the themselves
	// the depth is disabled after the draw 
	// in case you need to draw overlays
	glEnable(GL_DEPTH_TEST);

	glDrawArrays(GL_LINES, NUM_AXIS_POINTS + ARM_POINT_SIZE * index, ARM_POINT_SIZE);

	glDisable(GL_DEPTH_TEST);
}

void passToShader(float view[16], float model[16]) {
	GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
	glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, model);

	GLuint viewMatrix = glGetUniformLocationARB(program, "projection_matrix");
	glUniformMatrix4fv(viewMatrix, 1, GL_FALSE, view);

	GLuint LightPosition = glGetUniformLocationARB(program, "LightPosition");
	glUniform4fv(LightPosition, 1, lightPos);

	GLuint SpotLightPosition = glGetUniformLocationARB(program, "SpotLightPosition");
	glUniform4fv(SpotLightPosition, 1, spotLightPos);

	GLuint SpotLightDirection = glGetUniformLocationARB(program, "SpotLightDirection");
	glUniform4fv(SpotLightDirection, 1, spotLightDirection);

	GLuint SpotLightAngle = glGetUniformLocationARB(program, "SpotLightAngle");
	glUniform1f(SpotLightAngle, spotLightAngle);
}

//----------------------------------------------------------------------------

// keyboard handler
void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 033:
		exit(EXIT_SUCCESS);
		break;
	case 'e':	//Draw extents boxes
		isShowingExtents = !isShowingExtents;
		glutPostRedisplay();
		break;
	case 's':	//Toggle sinusoid movement
		isFollowingSinusoid = !isFollowingSinusoid;
		break;
	case 'p':	//increase spotlight cut off angle
		spotLightAngle = min(spotLightAngle + 0.5f, 89.0f);
		cout << "Spotlight Angle: " << spotLightAngle << "\n";
		break;
	case 'P':	//Decrease spotlight cut off angle
		spotLightAngle = max(spotLightAngle - 0.5f, 0.0f);
		cout << "Spotlight Angle: " << spotLightAngle << "\n";
		break;
	case 'm':	//Per-pixel lighting (smooth shading)
		if (isUsingFlatShading) {
			isUsingFlatShading = false;
			swapShaderPrograms(programSmooth);
		}
		break;
	case 'M':	//Flat shading
		if (!isUsingFlatShading) {
			isUsingFlatShading = true;
			swapShaderPrograms(programFlat);
		}
		break;
	}
}

void mouse(int button, int state, int x, int y) {}

void reshape(GLsizei w, GLsizei h) {
	width = (GLint)w;
	height = (GLint)h;
}

void idle(void) {
	frames++;
	int startTime = glutGet(GLUT_ELAPSED_TIME);

	if (isRotating) {
		vec3 r;

		//Rotate children
		for (int i = 0; i < NUM_PLY_FILES; i++) {
			r = mesh[i]->getLocalRotation();
			mesh[i]->setLocalRotation(r.x, fmod(r.y + MODEL_THETA * 3, 360.0f), r.z);
		}

		//Rotate arms
		for (int i = 0; i < 3; i++) {
			r = arm_mesh[i].getLocalRotation();
			arm_mesh[i].setLocalRotation(r.x, fmod(r.y - MODEL_THETA, 360.0f), r.z);
		}
	}
	if (isFollowingSinusoid) {
		sinusoid = fmod(sinusoid + MODEL_THETA * 2, 360.0f);

		for (int i = 1; i < NUM_PLY_FILES; i++) {
			empty_mesh[i].setLocalPosition(vec3(empty_mesh[i].getLocalPosition().x, 
												-MODEL_OFFSET + 0.5f * cos(sinusoid * DegreesToRadians), 
												empty_mesh[i].getLocalPosition().z));
		}
		for (int i = 0; i < 3; i++) {
			int offset = NUM_AXIS_POINTS + i * ARM_POINT_SIZE;
			points[offset + 5] = point4(arm_mesh[i].getVertices()[5].x,
				-MODEL_OFFSET + 0.5f * cos(sinusoid * DegreesToRadians),
				arm_mesh[i].getVertices()[5].z, 1.0f);

			points[offset + 7] = point4(arm_mesh[i].getVertices()[7].x,
				-MODEL_OFFSET + 0.5f * cos(sinusoid * DegreesToRadians),
				arm_mesh[i].getVertices()[7].z, 1.0f);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * numVertices, points);
		}
	}
	
	glutPostRedisplay();

	int endTime = glutGet(GLUT_ELAPSED_TIME);

	if (endTime - startTime < 1000 / FRAME_RATE) {
		Sleep(1000 / FRAME_RATE - (endTime - startTime));
	}
}

// Return the point at the center of the face
point4 calculateCenterPoint(point4 a, point4 b, point4 c) {
	return (a + b + c) / 3;
}


