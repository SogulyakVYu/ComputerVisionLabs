#pragma once
#include <vector>
// Интересная точка на изображении
class KeyPoint
{
private:
public:
	int x, y;
	double f;
	double angle;
	KeyPoint(int x, int y, double f, double angle);
	KeyPoint(int x, int y, double f);
	KeyPoint(int x, int y); 
	KeyPoint();
	KeyPoint(const KeyPoint& other);

	// Расстояние между точками
	static double distance(KeyPoint a, KeyPoint b);
};