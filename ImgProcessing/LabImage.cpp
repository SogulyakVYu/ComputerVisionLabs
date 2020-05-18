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
	drawKeyPoints(sourceImage, points, color);
}

void LabImage::save(const QString& fileName)
{
	sourceImage.save(fileName);
}

DoubleMatrix LabImage::getDoubleMatrix()
{
	return IntMatrix::fromImage(getGrayScale()).toDoubleMatrix();
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

void LabImage::drawKeyPoints(QImage& img1, const std::vector<KeyPoint>& points, QColor color, int radius)
{
	QPainter paint(&img1);
	paint.setPen(color);
	for (auto p : points) {
		paint.drawEllipse(QPoint(p.x, p.y), radius, radius);
	}
	paint.end();
}

void LabImage::drawKeyPoints(QImage& img1, const std::vector<KeyPoint>& points, std::vector<QColor>& colors, int radius)
{
	QPainter paint(&img1);
	int iColor = 0;
	for (auto p : points) {
		paint.setPen(colors[iColor++]);
		paint.drawEllipse(QPoint(p.x, p.y), radius, radius);
	}
}

void LabImage::drawMatches(QImage& img, int offsetX, int offsetY, std::vector<std::pair<int, int>>& matches, const std::vector<KeyPoint>& a, const std::vector<KeyPoint>& b, const std::vector<QColor>& colors)
{
	QPainter painter(&img);
	int iMatch = 0;
	for (auto& m : matches) {
		KeyPoint pointA = a[m.first];
		KeyPoint pointB = b[m.second];

		painter.setPen(colors[iMatch]);
		painter.drawEllipse(QPoint(pointA.x, pointA.y), 3, 3);
		painter.drawEllipse(QPoint(pointB.x + offsetX, pointB.y + offsetY), 3, 3);
		painter.setPen(QPen(colors[iMatch], 2));
		painter.drawLine(pointA.x, pointA.y, pointB.x + offsetX, pointB.y + offsetY);
		iMatch++;
	}
}

QImage LabImage::joinImages(QImage& img1, QImage& img2, int axis)
{
	int resultWidth = 0;
	int resultHeight = 0;
	int img2OffsetX = 0;
	int img2OffsetY = 0;
	if (axis == 1) {
		resultWidth = img1.width() + img2.width();
		resultHeight = std::max(img1.height(), img2.height());
		img2OffsetX = img1.width();
	}
	else if (axis == 0) {
		resultHeight = img1.height() + img2.height();
		resultWidth = std::max(img1.width(), img2.width());
		img2OffsetY = img1.height();
	}
	QImage result(resultWidth, resultHeight, img1.format());
	QPainter painter(&result);

	painter.drawImage(0, 0 , img1);
	painter.drawImage(img2OffsetX, img2OffsetY, img2);

	return result;
}

std::vector<QColor> LabImage::getRandomColors(int count)
{
	std::vector<QColor> colors;
	float currentHue = 0.0;
	for (int i = 0; i < count; i++) {
		colors.push_back(QColor::fromHslF(currentHue, 0.7, 0.5));
		currentHue += 0.618033988749895f;
		currentHue = std::fmod(currentHue, 1.0f);
	}
	return colors;
}
