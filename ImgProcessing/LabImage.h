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
	// Вывод размера и формата изображения
	void printInfo();
	// Возвращает копию изображения в оттенках серого
	QImage getGrayScale();

	// Выделить интересные точки на изображении  
	void drawKeyPoints(const std::vector<KeyPoint>& points, QColor color);
	void save(const QString& fileName);

	static IntMatrix createIntMatrixFromImage(QImage& source, char channel);
	// Возвращает копию изображения в оттенках серого
	static QImage getGrayScale(QImage& source);
	// Создает изображение из заданной матрицы 
	static QImage getImageFromMatrix(const IntMatrix& matrix);
	static QImage getImageFromMatrix(const DoubleMatrix& matrix);
	// Сохраняет изображение из заданной матрицы
	static void saveImage(IntMatrix& matrix, const QString& fileName);
	static void saveImage(DoubleMatrix& matrix, const QString& fileName);
};

