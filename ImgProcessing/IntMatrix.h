#pragma once
#include <QtGui>
#include <vector>

#include "DoubleMatrix.h"

class IntMatrix
{
private:
	// Основной вектор со значениями яркости избражения
	std::vector<int> matrix;

	// Высота матрицы
	int height;
	// Ширина матрицы
	int width;

public:
	IntMatrix(int w, int h);

	int getWidth() const { return width; }
	int getHeight() const { return height; }

	DoubleMatrix toDoubleMatrix();
	QImage toImage();
	void saveImage(const QString& fileName);
	void fillMatrix(int val);
	int get(int i, int j) const;
	void set(int i, int j, int val);
	void printMatrix() const;
	// Создание матрицы из указанного канала изображения 
	static IntMatrix fromImage(QImage& source, char channel = 'r');
	static IntMatrix fromDoubleMatrix(const DoubleMatrix& matrix);
};

