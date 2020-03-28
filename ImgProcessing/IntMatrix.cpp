#include "IntMatrix.h"
#include <iostream>

IntMatrix::IntMatrix(int w, int h)
{ 
	width = w;
	height = h;
	matrix.resize(width * height);
	std::fill(begin(matrix), end(matrix), 0);
}

DoubleMatrix IntMatrix::toDoubleMatrix()
{
	DoubleMatrix result(width, height);
	for (int i = 0; i < width * height; i++) {
		result.set(i, matrix[i]);
	}

	return result;
}

QImage IntMatrix::toImage()
{
	int h = height;
	int w = width;
	QImage resultImage(w, h, QImage::Format::Format_RGB32);

	for (int i = 0; i < h; i++) {
		uchar* scan = resultImage.scanLine(i);
		int offset = 4;
		for (int j = 0; j < w; j++) {
			QRgb* pixel = reinterpret_cast<QRgb*>(scan + j * offset);
			int pixelColor = get(i, j);
			*pixel = QColor(pixelColor, pixelColor, pixelColor).rgba();
		}
	}

	return resultImage;
}

void IntMatrix::saveImage(const QString& fileName)
{
	toImage().save(fileName);
}

void IntMatrix::fillMatrix(int val)
{
	fill(begin(matrix), end(matrix), val);
}


int IntMatrix::get(int i, int j) const
{
	return matrix[i * width + j];
}

void IntMatrix::set(int i, int j, int val)
{
	matrix[i * width + j] = val;
}

void IntMatrix::printMatrix() const
{
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			std::cout << matrix[i * width + j] << " ";
		}
		std::cout << std::endl;
	}
}

IntMatrix IntMatrix::fromImage(QImage& source, char channel)
{
	int h = source.height();
	int w = source.width();
	IntMatrix matrix(w, h);
	for (int i = 0; i < h; i++) {
		uchar* sourceScan = source.scanLine(i);
		int offset = 4;
		for (int j = 0; j < w; j++) {
			QRgb* pixel = reinterpret_cast<QRgb*>(sourceScan + j * offset);
			int channelVal = 0;
			if (channel == 'r') channelVal = qRed(*pixel);
			else if (channel == 'g') channelVal = qGreen(*pixel);
			else if (channel == 'b') channelVal = qBlue(*pixel);
			else if (channel == 'a') channelVal = qAlpha(*pixel);

			matrix.set(i, j, channelVal);
		}
	}

	return matrix;
}

IntMatrix IntMatrix::fromDoubleMatrix(const DoubleMatrix& matrix)
{
	int height = matrix.getHeight();
	int width = matrix.getWidth();
	IntMatrix result(width, height);
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			result.set(i, j, matrix.get(i, j));
		}
	}

	return result;
}
