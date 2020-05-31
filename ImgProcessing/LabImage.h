#pragma once
#include <QtGui>

#include "IntMatrix.h"
#include "KeyPoint.h"

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
	DoubleMatrix getDoubleMatrix();

	static IntMatrix createIntMatrixFromImage(QImage& source, char channel);
	// Возвращает копию изображения в оттенках серого
	static QImage getGrayScale(QImage& source);
	// Создает изображение из заданной матрицы 
	static QImage getImageFromMatrix(const IntMatrix& matrix);
	static QImage getImageFromMatrix(const DoubleMatrix& matrix);
	// Сохраняет изображение из заданной матрицы
	static void saveImage(IntMatrix& matrix, const QString& fileName);
	static void saveImage(DoubleMatrix& matrix, const QString& fileName);

	static void drawKeyPoints(QImage& img1, const std::vector<KeyPoint>& points, QColor color, int radius = 2);
	static void drawKeyPoints(QImage& img1, const std::vector<KeyPoint>& points, std::vector<QColor>& colors, int radius = 2);
	static void drawKeyPointAngle(QImage& img1, const std::vector<KeyPoint>& points, QColor color);
	static void drawMatches(QImage& img, int offsetX, int offsetY, std::vector<std::pair<int, int>>& matches, const std::vector<KeyPoint>& a, const std::vector<KeyPoint>& b, const std::vector<QColor>& colors);
	static QImage joinImages(QImage& img1, QImage& img2, int axis = 1);
	static std::vector<QColor> getRandomColors(int count);
};

