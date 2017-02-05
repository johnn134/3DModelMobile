/*
 * Polygon.cpp
 * author: John Nelson (jpnelson@wpi.edu)
 */

#include "Polygon.h"
#include <iostream>

using namespace Angel;

MyMesh::MyMesh() {
	faces = NULL; 
	numFaces = 0; 
}

MyMesh::MyMesh(point4 *_vertices, int numV, vec3 *_faces, int numF, bool _isDynamic) {
	vertices = _vertices;
	numVertices = numV;
	faces = _faces;
	numFaces = numF;
	isDynamic = _isDynamic;

	calculateUpperBounds();
	calculateLowerBounds();

	center = (upperBounds + lowerBounds) / 2.0f;

	if (vertices != NULL && faces != NULL) {
		normals = new point4[numV];
		calculateVertexNormals();
	}
}

MyMesh::~MyMesh() {
	if (isDynamic) {
		if (vertices != NULL)
			delete(vertices);
		if (faces != NULL)
			delete(faces);
	}
	if(normals != NULL)
		delete(normals);
}

void MyMesh::calculateUpperBounds() {
	upperBounds = vec3(-100000, -100000, -100000);

	for (int i = 0; i < numVertices; i++) {
		//X
		if (vertices[i].x > upperBounds.x)
			upperBounds.x = vertices[i].x;
		//Y
		if (vertices[i].y > upperBounds.y)
			upperBounds.y = vertices[i].y;
		//Z
		if (vertices[i].z > upperBounds.z)
			upperBounds.z = vertices[i].z;
	}
}

void MyMesh::calculateLowerBounds() {
	//Lower bounds
	lowerBounds = vec3(100000, 100000, 100000);

	for (int i = 0; i < numVertices; i++) {
		//X
		if (vertices[i].x < lowerBounds.x)
			lowerBounds.x = vertices[i].x;
		//Y
		if (vertices[i].y < lowerBounds.y)
			lowerBounds.y = vertices[i].y;
		//Z
		if (vertices[i].z < lowerBounds.z)
			lowerBounds.z = vertices[i].z;
	}
}

void MyMesh::calculateVertexNormals() {
	for (int i = 0; i < numVertices; i++) {
		point4 sumFaceNormals = point4(0);

		for (int j = 0; j < numFaces; j++) {
			if (faces[j].x == i || faces[j].y == i || faces[j].z == i)
				sumFaceNormals = sumFaceNormals + calculateNormal(getFaceX(j), getFaceY(j), getFaceZ(j));
		}
		sumFaceNormals.w = 1.0;

		normals[i] = normalize(sumFaceNormals);
	}
}

void MyMesh::setParent(MyMesh *_parent) {
	parent = _parent;
	if (parent != NULL) {
		if (parent->isRoot()) {
			setLocalPosition(position - parent->getPosition());
			setLocalRotation(rotation - parent->getRotation());
		}
		else {
			setLocalPosition(position - parent->getLocalPosition());
			setLocalRotation(rotation - parent->getLocalRotation());
		}
	}
}

vec3 MyMesh::getScaleToDimensions(float size) {
	vec3 sca = vec3(0.0f);

	if (this->getHeight() > this->getWidth()
		&& this->getHeight() > this->getDepth()) {	//Tall
		sca = vec3(size / this->getHeight());
	}
	else {	//wide
		if (this->getWidth() > this->getDepth()) {	//Larger X
			sca = vec3(size / this->getWidth());
		}
		else {	//Larger Z
			sca = vec3(size / this->getDepth());
		}
	}

	return sca;
}

vec3 MyMesh::getWorldPosition() {
	return parent == NULL ? position : parent->getWorldPosition() + localPosition;
}

vec3 MyMesh::getWorldRotation() {
	return parent == NULL ? rotation : parent->getWorldRotation() + localRotation;
}

mat4 MyMesh::getCTM() {
	if (parent == NULL) {
		return Angel::Translate(position)
			* Angel::Translate(center * scale)
			* Angel::RotateZ(rotation.z)
			* Angel::RotateY(rotation.y)
			* Angel::RotateX(rotation.x)
			* Angel::Translate(center * scale * -1.0f)
			* Angel::Scale(scale);
	}
	else {
		return parent->getParentCTM()
			* Angel::Translate(localPosition)
			* Angel::Translate(center * scale)
			* Angel::RotateZ(localRotation.z)
			* Angel::RotateY(localRotation.y)
			* Angel::RotateX(localRotation.x)
			* Angel::Translate(center * scale * -1.0f)
			* Angel::Scale(scale);
	}
}

mat4 MyMesh::getParentCTM() {
	if (parent == NULL) {
		return Angel::Translate(position)
			* Angel::Translate(center * scale)
			* Angel::RotateZ(rotation.z)
			* Angel::RotateY(rotation.y)
			* Angel::RotateX(rotation.x)
			* Angel::Translate(center * scale * -1.0f);
	}
	else {
		return parent->getParentCTM()
			* Angel::Translate(localPosition)
			* Angel::Translate(center * scale)
			* Angel::RotateZ(localRotation.z)
			* Angel::RotateY(localRotation.y)
			* Angel::RotateX(localRotation.x)
			* Angel::Translate(center * scale * -1.0f);
	}
}

point4 MyMesh::calculateNormal(point4 a, point4 b, point4 c) {
	point4 normal = point4(0, 0, 0, 1);

	normal.x = (a.y - b.y) * (a.z + b.z)
		+ (b.y - c.y) * (b.z + c.z)
		+ (c.y - a.y) * (c.z + a.z);
	normal.y = (a.z - b.z) * (a.x + b.x)
		+ (b.z - c.z) * (b.x + c.x)
		+ (c.z - a.z) * (c.x + a.x);
	normal.z = (a.x - b.x) * (a.y + b.y)
		+ (b.x - c.x) * (b.y + c.y)
		+ (c.x - a.x) * (c.y + a.y);

	return normalize(normal);
}
