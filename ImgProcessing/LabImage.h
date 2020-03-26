#pragma once
#include <QtGui>

#include "IntMatrix.h"

class LabImage
{
private:
	QImage& sourceImage;
	QImage::Format imageFormat;
	int width;
	int height;
public:
	LabImage(QImage& source);
	void printInfo();
	QImage getGrayScale();

	static IntMatrix createIntMatrixFromImage(QImage& source, char channel);
	static QImage getGrayScale(QImage& source);
	static QImage getImageFromMatrix(const IntMatrix& matrix);
	static QImage getImageFromMatrix(const DoubleMatrix& matrix);
	static void saveImage(const IntMatrix& matrix, const QString& fileName);
	static void saveImage(const DoubleMatrix& matrix, const QString& fileName);
};

