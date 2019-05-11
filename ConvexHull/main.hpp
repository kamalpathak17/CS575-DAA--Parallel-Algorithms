#ifndef _open_
#define _open_


#include <stdlib.h>
#include <iostream>
#include <omp.h>
#include <stack>
#include <time.h>
#include <chrono>
#include <assert.h>
#include <vector>
#include <math.h>

struct Point { int x, y; };
 
Point p0;

std::stack<Point> convexHull(std::vector<Point> points, int );
std::stack<Point> convexHull_1(std::vector<Point> points, int );
void printOutPut(std::stack<Point> &S);

#endif