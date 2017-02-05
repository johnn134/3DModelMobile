#ifndef POLYGON_H
#define POLYGON_H

/*
* Polygon.h
* author: John Nelson (jpnelson@wpi.edu)
*/


#include "Angel.h"

using namespace Angel;

typedef vec4 point4;

class MyMesh {
private:
	point4 *vertices;
	vec3 *faces;

	point4 *normals = NULL;

	MyMesh *parent = NULL;

	int numVertices, numFaces;
	int startingIndex = 0;
	int startingNormalsIndex = 0;

	vec3 scale = vec3(1.0f);
	vec3 localScale = vec3(1.0f);

	vec3 position = vec3(0.0f);
	vec3 localPosition = vec3(0.0f);

	vec3 rotation = vec3(0.0f);
	vec3 localRotation = vec3(0.0f);

	vec3 upperBounds, lowerBounds, center;

	bool isDynamic = true;

	void calculateUpperBounds();
	void calculateLowerBounds();
	void calculateVertexNormals();

public:
	MyMesh();
	MyMesh(point4 *_vertices, int numV, vec3 *_faces, int numF, bool _isDynamic);
	~MyMesh();

	void setParent(MyMesh *_parent);
	MyMesh *getParent() { return parent; }

	bool isRoot() { return parent == NULL; }

	point4 *getVertices() { return vertices; }
	vec3 *getFaces() { return faces; }
	point4 *getNormals() { return normals; }

	int getNumFaces() { return numFaces; }
	int getNumVertices() { return numVertices; }

	void setStartingIndex(int index) { startingIndex = index; }
	int getStartingIndex() { return startingIndex; }

	int getTotalVertices() { return numFaces * 3; }

	point4 getFaceX(int index) { return vertices[(int)faces[index].x]; }
	point4 getFaceY(int index) { return vertices[(int)faces[index].y]; }
	point4 getFaceZ(int index) { return vertices[(int)faces[index].z]; }

	vec3 getUpperBounds() { return upperBounds; }
	vec3 getLowerBounds() { return lowerBounds; }
	vec3 getCenter() { return center; }

	float getWidth() { return upperBounds.x - lowerBounds.x; }
	float getHeight() { return upperBounds.y - lowerBounds.y; }
	float getDepth() { return upperBounds.z - lowerBounds.z; }

	void setRotation(vec3 _rotation) { rotation = _rotation; }
	void setRotation(float _x, float _y, float _z) { rotation = vec3(_x, _y, _z); }
	vec3 getRotation() { return rotation; }

	void setLocalRotation(vec3 _rotation) { localRotation = _rotation; }
	void setLocalRotation(float _x, float _y, float _z) { localRotation = vec3(_x, _y, _z); }
	vec3 getLocalRotation() { return localRotation; }

	void setPosition(vec3 _position) { position = _position; }
	void setPosition(float _x, float _y, float _z) { position = vec3(_x, _y, _z); }
	vec3 getPosition() { return position; }

	void setPositionToOrigin() { position = center * scale * -1.0f; }

	void setLocalPosition(vec3 _position) { localPosition = _position; }
	void setLocalPosition(float _x, float _y, float _z) { localPosition = vec3(_x, _y, _z); }
	vec3 getLocalPosition() { return localPosition; }

	void setScale(vec3 _scale) { scale = _scale; }
	void setScale(float _x, float _y, float _z) { scale = vec3(_x, _y, _z); }
	vec3 getScale() { return scale; }

	void setLocalScale(vec3 _scale) { localScale = _scale; }
	void setLocalScale(float _x, float _y, float _z) { localScale = vec3(_x, _y, _z); }
	vec3 getLocalScale() { return localScale; }

	vec3 getWorldPosition();
	vec3 getWorldRotation();

	mat4 getCTM();
	mat4 getParentCTM();

	vec3 getScaleToDimensions(float size);

	point4 calculateNormal(point4 a, point4 b, point4 c);
};

#endif
