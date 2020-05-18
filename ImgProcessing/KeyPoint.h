#pragma once
#include <vector>
// Интересная точка на изображении
class KeyPoint
{
private:
public:
	int x, y;
	double f;
	KeyPoint(int x, int y, double f);
	KeyPoint(int x, int y); 
	KeyPoint();

	// Расстояние между точками
	static double distance(KeyPoint a, KeyPoint b);
};