#include "LabImage.h"
#include <iostream>

LabImage::LabImage(QImage& source): sourceImage(source)
{
	sourceImage = source;
	width = source.width();
	height = source.height();
	imageFormat = source.format();
}

void LabImage::printInfo()
{
	std::cout << "Size: " << width << "x" << height << "(" << width * height << ") ";
	std::cout << "Format: " << sourceImage.format() << std::endl;
}

QImage LabImage::getGrayScale()
{
	return getGrayScale(sourceImage);
}

void LabImage::drawKeyPoints(const std::vector<KeyPoint>& points, QColor color)
{
	QPainter paint(&sourceImage);
	paint.setPen(color);
	for (auto p : points) {
		paint.drawEllipse(QPoint(p.x, p.y), 2, 2);
	}
	paint.end();
}

void LabImage::save(const QString& fileName)
{
	sourceImage.save(fileName);
}

IntMatrix LabImage::createIntMatrixFromImage(QImage& source, char channel)
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

QImage LabImage::getGrayScale(QImage& source)
{
	QImage resultImage(source.width(), source.height(), source.format());
	double rCoef = 0.2126;
	double gCoef = 0.7152;
	double bCoef = 0.0722;

	for (int j = 0; j < resultImage.height(); j++) {
		uchar* sourceScan = source.scanLine(j);
		uchar* resultScan = resultImage.scanLine(j);
		int offset = 4;
		for (int i = 0; i < resultImage.width(); i++) {
			QRgb* pixel = reinterpret_cast<QRgb*>(sourceScan + i * offset);
			QRgb* resultPixel = reinterpret_cast<QRgb*>(resultScan + i * offset);
			int r = qRed(*pixel);
			int g = qGreen(*pixel);
			int b = qBlue(*pixel);
			double y = rCoef * r + gCoef * g + bCoef * b;
			QColor gray(y, y, y);
			*resultPixel = gray.rgba();
		}
	}

	return resultImage;
}

QImage LabImage::getImageFromMatrix(const IntMatrix& matrix)
{
	int h = matrix.getHeight();
	int w = matrix.getWidth();
	QImage resultImage(w, h, QImage::Format::Format_RGB32);

	for (int i = 0; i < h; i++) {
		uchar* scan = resultImage.scanLine(i);
		int offset = 4;
		for (int j = 0; j < w; j++) {
			QRgb* pixel = reinterpret_cast<QRgb*>(scan + j * offset);
			int pixelColor = matrix.get(i, j);
			*pixel = QColor(pixelColor, pixelColor, pixelColor).rgba();
		}
	}

	return resultImage;
}

QImage LabImage::getImageFromMatrix(const DoubleMatrix& matrix)
{
	int h = matrix.getHeight();
	int w = matrix.getWidth();
	QImage resultImage(w, h, QImage::Format::Format_RGB32);

	for (int i = 0; i < h; i++) {
		uchar* scan = resultImage.scanLine(i);
		int offset = 4;
		for (int j = 0; j < w; j++) {
			QRgb* pixel = reinterpret_cast<QRgb*>(scan + j * offset);
			int pixelColor = matrix.get(i, j);
			*pixel = QColor(pixelColor, pixelColor, pixelColor).rgba();
		}
	}

	return resultImage;
}

void LabImage::saveImage(IntMatrix& matrix, const QString& fileName)
{
	getImageFromMatrix(matrix).save(fileName);
}

void LabImage::saveImage(DoubleMatrix& matrix, const QString& fileName)
{
	DoubleMatrix m = DoubleMatrix(matrix);
	getImageFromMatrix(m.norm255()).save(fileName);
}
