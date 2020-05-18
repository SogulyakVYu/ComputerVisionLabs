#include <algorithm>
#include <QtCore\qdebug.h>
#include "KeyPoint.h"

KeyPoint::KeyPoint(int x, int y, double f): x(x), y(y), f(f) {}

KeyPoint::KeyPoint(int x, int y): KeyPoint(x, y, 0.0) {}

KeyPoint::KeyPoint(): KeyPoint(0, 0, 0.0) {}

double KeyPoint::distance(KeyPoint a, KeyPoint b)
{
	double coordX = a.x - b.x;
	double coordY = a.y - b.y;
	return sqrt(coordX * coordX + coordY * coordY);
}
