/*
 * FileReader.cpp
 * author: John Nelson (jpnelson@wpi.edu)
 */

#include "FileReader.h"
#include "textfile.h"
#include <iostream>

using namespace std;

MyMesh *readPLYFile(char *filename) {
	char *contents = textFileRead(filename);

	cout << "Reading " << filename << "\n";

	int iter = 0;

	//cout << "Char: \"" << contents[iter] << "\"\n";

	/*** PLY Header ***/

	char header[4];
	memcpy(header, contents, 3);
	header[3] = '\0';

	if (strcmp(header, "ply"))	//Check for ply header
		return NULL;

	nextLine(contents, &iter);
	nextLine(contents, &iter);

	/*** Number of Vertices ***/

	//skip to number
	skipToSpace(contents, &iter); iter++;
	skipToSpace(contents, &iter); iter++;

	int numVertices = (int)getDouble(contents, &iter);

	//cout << "Num vert: " << numVertices << "\n";

	nextLine(contents, &iter);	//skip past # vertices
	nextLine(contents, &iter);
	nextLine(contents, &iter);
	nextLine(contents, &iter);

	/*** Number of Faces ***/

	//skip to number
	skipToSpace(contents, &iter); iter++;
	skipToSpace(contents, &iter); iter++;

	int numFaces = (int)getDouble(contents, &iter);

	//cout << "Num poly: " << numFaces << "\n";

	nextLine(contents, &iter);	//skip past # faces
	nextLine(contents, &iter);
	nextLine(contents, &iter);

	/*** Read each Vertex ***/

	point4 *vertices = new point4[numVertices];

	for (int i = 0; i < numVertices; i++) {
		double x = getDouble(contents, &iter); iter++;
		double y = getDouble(contents, &iter); iter++;
		double z = getDouble(contents, &iter);

		vertices[i] = point4((GLfloat)x, (GLfloat)y, (GLfloat)z, 1.0);

		nextLine(contents, &iter);
	}

	/*** Read each Polygon ***/

	vec3 *faces = new vec3[numFaces];

	for (int i = 0; i < numFaces; i++) {
		//Skip first number
		skipToSpace(contents, &iter); iter++;

		//Read verticies in polygon
		double a = getDouble(contents, &iter); iter++;
		double b = getDouble(contents, &iter); iter++;
		double c = getDouble(contents, &iter);

		faces[i] = vec3((GLfloat)a, (GLfloat)b, (GLfloat)c);

		nextLine(contents, &iter);
	}

	/*** Make Mesh ***/

	MyMesh *m = new MyMesh(vertices, numVertices, faces, numFaces, true);

	return m;
}

void skipDelimiters(char *arr, int *iter) {
	while (arr[*iter] == ' ' || arr[*iter] == '\n')
		(*iter)++;
}

void nextLine(char *arr, int *iter) {
	while (arr[*iter] != '\n')
		(*iter)++;
	
	//skip newline
	(*iter)++;
}

void skipToSpace(char *arr, int *iter) {
	while (arr[*iter] != ' ')
		(*iter)++;
}

double getDouble(char *arr, int *iter) {
	int start = (*iter);

	while (arr[*iter] != ' ' && arr[*iter] != '\n') {
		(*iter)++;
	}

	int length = (*iter) - start;

	char *word = NULL;
	word = (char *)malloc(sizeof(char) * (length + 1));
	word[length] = '\0';

	double d = atof(strncpy(word, arr + start, length));

	delete(word);

	return d;
}