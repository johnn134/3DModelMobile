/*
 * FileReader.h
 * author: John Nelson (jpnelson@wpi.edu)
 */

#ifndef FILEREADER_H
#define FILEREADER_H

#include "Polygon.h"

MyMesh *readPLYFile(char *filename);
void skipDelimiters(char *arr, int *iter);
void nextLine(char *arr, int *iter);
void skipToSpace(char *arr, int *iter);
double getDouble(char *arr, int *iter);


#endif