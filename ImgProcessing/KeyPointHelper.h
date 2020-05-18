#pragma once
#include "DoubleMatrix.h"
#include "KeyPoint.h"
class KeyPointHelper
{
public:
	// Фильтрация точек методом Adaptive Non-Maximum Suppression
	static std::vector<KeyPoint> anms(std::vector<KeyPoint>& points, int pointsCount, double minR = 1, double maxR = 100);
	static std::vector<KeyPoint> brownAnms(std::vector<KeyPoint>& points, int pointCount);
	// Возращает набор интересных точек соответствующих локальным максимумам выше заданного порога
	static std::vector<KeyPoint> getLocalMax(const DoubleMatrix& img, const int windowSize, double threshold);
	static std::vector<KeyPoint> getLocalMax(const DoubleMatrix& img, const std::vector<int>& windowSize, double threshold);
	// Возвращает набор интересных точек, значения которых выше порога
	static std::vector<KeyPoint> getKeyPoints(const DoubleMatrix& img, double threshold);
};

