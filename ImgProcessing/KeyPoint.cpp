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

std::vector<KeyPoint> KeyPoint::anms(std::vector<KeyPoint>& points,int pointsCount, double minR, double maxR)
{
	double r = minR;
	double step = 1;
	while (points.size() > pointsCount && r < maxR)
	{
		auto toRemove = std::remove_if(begin(points), end(points), 
			[&](KeyPoint& pointA) {
				for (auto& pointB : points) {
					if (distance(pointA, pointB) < r && pointA.f < pointB.f) {
						return true;
					}
				}
				return false;
			});
		points.erase(toRemove, end(points));
		r += step;
	}

	return points;
}
