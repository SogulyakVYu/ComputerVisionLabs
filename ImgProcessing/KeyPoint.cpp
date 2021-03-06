#include <algorithm>
#include <QtCore\qdebug.h>
#include "KeyPoint.h"

KeyPoint::KeyPoint(int x, int y, double f, double angle, double sigma) : x(x), y(y), f(f), angle(angle), sigma(sigma) {}

KeyPoint::KeyPoint(int x, int y, double f, double angle) : KeyPoint(x, y, f, angle, 0.0) {}

KeyPoint::KeyPoint(int x, int y, double f): KeyPoint(x, y, f, 0.0) {}

KeyPoint::KeyPoint(int x, int y): KeyPoint(x, y, 0.0) {}

KeyPoint::KeyPoint(): KeyPoint(0, 0, 0.0) {}

KeyPoint::KeyPoint(const KeyPoint& other) : KeyPoint(other.x, other.y, other.f, other.angle, other.sigma) {}

double KeyPoint::distance(KeyPoint a, KeyPoint b)
{
	double coordX = a.x - b.x;
	double coordY = a.y - b.y;
	return sqrt(coordX * coordX + coordY * coordY);
}
