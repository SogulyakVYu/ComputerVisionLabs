#pragma once
#include <qstring.h>
#include "DoubleMatrix.h"
#include "KeyPoint.h"

struct PyramidRow {
	int octave;
	int level;
	double sigmaLocal;
	double sigmaEffective;
	DoubleMatrix image;
};

class Pyramid
{
private:
	// Число октав
	int octaveCount;
	// Число изображений в одной октаве
	int levelCount;
	// Число дополнитльеных изображений для DoG
	int overlapCount;
	// Шаг для сигмы
	double sigmaStep;
	// Начальное значние сигмы в пирамиде
	double sigma0;
	// Список изображений
	std::vector<PyramidRow> pyramid;
	// Индексы изображений по увеличению значения sigmaEffective, для поиска изображений по сигме
	std::vector<int> rowsBySigma;

	// Возвращает список соседних точек в окрестности из трех соседних изображений
	std::vector<double> getNeighbors3d(int x, int y, int iOctave, int iLevel, int winSize);

public:
	std::vector<PyramidRow>& get();
	PyramidRow& get(int i);
	PyramidRow& get(int octave, int level) { return pyramid[octave * levelCount + level]; }
	// Возвращает изображение ближайшее к заданной сигме
	PyramidRow& getBySigma(double sigma);
	// Возвращает из заданной октавы изображение ближайшее к заданной сигме
	PyramidRow& getBySigma(int octave, double sigma);
	DoubleMatrix& getImage(int i);
	DoubleMatrix& getImage(int octave, int level) { return get(octave, level).image; }
	int getOctaveCount() { return octaveCount; }
	int getLevelCount() { return levelCount; }
	int getOverlapCount() { return overlapCount; }
	double getSigmaStep() { return sigmaStep; }
	double getSigma0() { return sigma0; }

	PyramidRow& operator[](int i) { return pyramid[i]; }

	void saveImage(const QString& dir, int format = 0);
	double getPixel(int x, int y, double sigma);

	Pyramid createDoGPyramid();
	Pyramid createHarrisPyramid(int windowSize = 5);
	Pyramid createGradientPyramid();
	Pyramid createDirectionsPyramid();
	// Возвращает список экстремумов, которые больше заданного порога в DoG
	std::vector<KeyPoint> findExtremePoints(int winSize, double threshold = 0.03);
	// Создает пирамиду из заданного изображения
	static Pyramid createFrom(const DoubleMatrix& image, double sigmaA, double sigma0, int octaveCount, int levelCount);
	// Создает пирамиду из заданного изображения с дополнительными, невходящими в октаву (для DoG)
	static Pyramid createWithOverlap(const DoubleMatrix& image, double sigmaA, double sigma0, int octaveCount, int levelCount, int overlap = 1);
};


