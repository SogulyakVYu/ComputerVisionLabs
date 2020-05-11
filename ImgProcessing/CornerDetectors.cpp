#include "DoubleMatrix.h"
#include <iostream>
#include <algorithm>
#include <QtCore\qmath.h>
#include <QtCore\qdebug.h>
#include "KeyPoint.h"

std::vector<KeyPoint> DoubleMatrix::getLocalMax(const int windowSize, double threshold) {
	return getLocalMax({ windowSize, windowSize }, threshold);
}

std::vector<KeyPoint> DoubleMatrix::getLocalMax(const std::vector<int>& windowSize, double threshold) {
	int offsetY = windowSize[0] / 2;
	int offsetX = windowSize[1] / 2;
	std::vector<KeyPoint> localMaxPoints;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			bool isLocalMax = true;
			double localMax = get(y, x);
			for (int u = -offsetY; u <= offsetY && isLocalMax; u++) {
				for (int v = -offsetX; v <= offsetX && isLocalMax; v++) {
					if (u != 0 || v != 0) {
						isLocalMax = localMax > get(y + u, x + v);
					}
				}
			}

			if (isLocalMax && localMax > threshold) {
				localMaxPoints.push_back(KeyPoint(x, y, localMax));
			}
		}
	}

	return localMaxPoints;
}

std::vector<KeyPoint> DoubleMatrix::getKeyPoints(double threshold) {

	std::vector<KeyPoint> output;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			double p = at(y, x);
			if (p > threshold) output.push_back(KeyPoint(x, y, p));
		}
	}
	return output;
}

double DoubleMatrix::moravecC(int x, int y, const std::vector<int>& windowSize, const std::vector<int>& d)
{
	int offsetY = windowSize[0] / 2;
	int offsetX = windowSize[1] / 2;
	double sum = 0;
	for (int u = -offsetY; u <= offsetY; u++) {
		for (int v = -offsetX; v <= offsetX; v++) {
			double diff = at(y + u, x + v) - at(y + u + d[0], x + v + d[1]);
			sum += diff * diff;
		}
	}

	return sum;
}

DoubleMatrix DoubleMatrix::operatorMoravec(int windowSize)
{
	DoubleMatrix* workImg = new DoubleMatrix(width, height);
    std::vector<int> wSize{ windowSize, windowSize };
	int offsetY = wSize[0] / 2 + 1;
	int offsetX = wSize[1] / 2 + 1;
	std::vector<int> d(2);
	copyWithBorder(*this, workImg, offsetX, offsetY);

	DoubleMatrix sValues(width, height);
	for (int y = offsetY; y < workImg->height - offsetY; y++) {
		for (int x = offsetX ; x < workImg->width - offsetX; x++) {
			double pointS = std::numeric_limits<double>::max();
			for (int i = -1; i <= 1; i++) {
				for (int j = -1; j <= 1; j++) {
					if (i != 0 || j != 0) {
						d[0] = i;
						d[1] = j;
						double err = workImg->moravecC(x, y, wSize, d);
						pointS = std::min(pointS, err);
					}
				}
			}
			sValues.set(y - offsetY, x - offsetX, pointS);
		}
	}
	return sValues;
}

DoubleMatrix DoubleMatrix::operatorHarris(int windowSize) {

	DoubleMatrix dx = this->dx();
	DoubleMatrix dy = this->dy();
	DoubleMatrix dx2 = dx * dx;
	DoubleMatrix dy2 = dy * dy;
	DoubleMatrix dxy = dx * dy;
	DoubleMatrix gauss = createGaussian(windowSize, windowSize, windowSize / 6.);

	DoubleMatrix a = dx2.convolution(gauss);
	DoubleMatrix b = dxy.convolution(gauss);
	DoubleMatrix c = dy2.convolution(gauss);

	return harrisE(a, b, c);
}

DoubleMatrix DoubleMatrix::harrisF(DoubleMatrix& a, DoubleMatrix& b, DoubleMatrix& c, double coef) {
	DoubleMatrix det = a * c - b * b;
	DoubleMatrix trace = a + c;
	return det - coef * trace * trace;
}

DoubleMatrix DoubleMatrix::harrisE(DoubleMatrix& a, DoubleMatrix& b, DoubleMatrix& c) {
	DoubleMatrix bx = a + c;
	DoubleMatrix cx = a * c - b * b;
	DoubleMatrix d = bx * bx - 4 * cx;
	DoubleMatrix result(a.width, b.height);
	for (int i = 0; i < a.matrix.size(); i++) {
		double l1 = (bx[i] + sqrt(d[i])) / 2;
		double l2 = (bx[i] - sqrt(d[i])) / 2;
		result[i] = std::min(l1, l2);
	}

	return result;
}