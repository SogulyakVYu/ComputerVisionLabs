#pragma once
#include <qstring.h>
#include "DoubleMatrix.h"

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
	int octaveCount;
	int levelCount;
	double sigmaStep;
	double sigma0;
	std::vector<PyramidRow> pyramid;

public:
	std::vector<PyramidRow>& get();
	PyramidRow& get(int i);
	PyramidRow& get(int octave, int level) { return pyramid[octave * levelCount + level]; }
	DoubleMatrix& getImage(int i);
	DoubleMatrix& getImage(int octave, int level) { return get(octave, level).image; }
	int getOctaveCount() { return octaveCount; }
	int getLevelCount() { return levelCount; }

	PyramidRow& operator[](int i) { return pyramid[i]; }

	void saveImage(const QString& dir);
	double getPixel(int x, int y, double sigma);
	static Pyramid createFrom(DoubleMatrix& image, double sigmaA, double sigma0, int octaveCount, int levelCount);
};


