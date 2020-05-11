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
	
	// Фильтрация точек методом Adaptive Non-Maximum Suppression
	static std::vector<KeyPoint> anms(std::vector<KeyPoint>&  points, int pointCount, double minR = 1, double maxR = 100);
};

