#include <QtCore/qmath.h>
#include <QtCore/qdebug.h>
#include "DescriptorExtractor.h"

std::pair<int, int> DescriptorExtractor::getBinsIndexies(double phi, double binSize, int binCount)
{
	int indexBin1 = phi / binSize;
	int indexBin2 = phi / binSize + 0.5;
	if (indexBin1 == indexBin2) {
		indexBin2 = indexBin1 - 1;
		indexBin2 = indexBin2 > -1 ? indexBin2 : binCount - 1;
	}
	else {
		indexBin2 = indexBin1 + 1;
		indexBin2 = indexBin2 < binCount ? indexBin2 : 0;
	}
	if (indexBin1 == binCount) return std::make_pair(0, indexBin2);
	return std::make_pair(indexBin1, indexBin2);
}

DescriptorExtractor::DescriptorExtractor(int gridSize, int cellCount, int histogramCount, int binCount):
	gridSize(gridSize), cellCount(cellCount), cellSize(gridSize/cellCount), histogramCount(histogramCount), binCount(binCount) {}

DescriptorExtractor::DescriptorExtractor(int gridSize, int cellCount, int binCount) :
	gridSize(gridSize), cellCount(cellCount), cellSize(gridSize / cellCount), histogramCount(cellCount * cellCount), binCount(binCount) {}

void DescriptorExtractor::fillDescriptor(Descriptor& descriptor, const DoubleMatrix& dirs, const DoubleMatrix& grad, KeyPoint& point)
{
	int radius = gridSize / 2;
	double binSize = 2 * M_PI / binCount;
	DoubleMatrix gauss = DoubleMatrix::createGaussian(gridSize + 1, gridSize + 1, gridSize / 6.);

	for (int i = -radius; i < radius; i++) {
		for (int j = -radius; j < radius; j++) {
			double phi = dirs.get(point.y + i, point.x + j);

			std::pair<int, int> binsIndex = getBinsIndexies(phi, binSize, binCount);
			double bin1Center = binsIndex.first * binSize + binSize / 2;
			double distToBin1Center = abs(bin1Center - phi);
			double distToBin2Center = binSize - distToBin1Center;
			int ii = i + radius;
			int jj = j + radius;
			int curHistogram = (ii / cellSize) * cellCount + (jj / cellSize);
			double gradVal = grad.get(point.y + i, point.x + j);
			descriptor.at(curHistogram, binsIndex.first) += gradVal * (1 - distToBin1Center / binSize) * gauss.at(ii, jj);
			descriptor.at(curHistogram, binsIndex.second) += gradVal * (1 - distToBin2Center / binSize) * gauss.at(ii, jj);
		}
	}
}

std::vector<Descriptor> DescriptorExtractor::compute(const DoubleMatrix& img, std::vector<KeyPoint>& points)
{
	DoubleMatrix gradient = img.calcSobel();
	DoubleMatrix gradientDirs = img.gradientDirection();
	std::vector<Descriptor> descriptors;
	gridPoints.clear();
	for (int i = 0; i < points.size(); i++) {
		descriptors.push_back(Descriptor(gridSize, cellCount, binCount));
		fillDescriptorAngle(descriptors[i], gradientDirs, gradient, points[i]);
		descriptors[i].normalize();
		descriptors[i].truncate(0.2);
		descriptors[i].normalize();
	}

	return descriptors;
}

void DescriptorExtractor::fillDescriptorAngle(Descriptor& descriptor, const DoubleMatrix& dirs, const DoubleMatrix& grad, KeyPoint& point)
{
	int radius = gridSize / 2;
	double binSize = 2 * M_PI / binCount;
	double twoPi = 2 * M_PI;
	DoubleMatrix gauss = DoubleMatrix::createGaussian(gridSize + 1, gridSize + 1, gridSize / 6.);

	for (int y = -radius; y < radius; y++) {
		for (int x = -radius; x < radius; x++) {
			double x1 = x * cos(point.angle) + y * sin(point.angle) + 0.5;
			double y1 = y * cos(point.angle) - x * sin(point.angle) + 0.5;

			if (y == -radius || x == -radius || y == radius - 1 || x == radius - 1) {
				gridPoints.push_back(KeyPoint(point.x + x1, point.y + y1, 0, point.angle));
			}

			if (y1 < -radius) y1 = -radius;
			else if (y1 > radius - 1) y1 = radius - 1;
			if (x1 < -radius) x1 = -radius;
			else if (x1 > radius - 1) x1 = radius - 1;
			
			double phi = dirs.get(point.y + y, point.x + x);
			phi = phi - point.angle;
			phi = phi < 0 ? phi + twoPi : phi;
			phi = phi > twoPi ? phi - twoPi : phi;

			std::pair<int, int> binsIndex = getBinsIndexies(phi, binSize, binCount);
			double bin1Center = binsIndex.first * binSize + binSize / 2;
			double distToBin1Center = abs(bin1Center - phi);
			double distToBin2Center = binSize - distToBin1Center;
			int iHist = y1 + radius;
			int jHist = x1 + radius;
			int curHistogram = (iHist / cellSize) * cellCount + (jHist / cellSize);
			double gradVal = grad.get(point.y + y, point.x + x);
			descriptor.at(curHistogram, binsIndex.first) += gradVal * (1 - distToBin1Center / binSize) * gauss.at(iHist, jHist);
			descriptor.at(curHistogram, binsIndex.second) += gradVal * (1 - distToBin2Center / binSize) * gauss.at(iHist, jHist);
		}
	}
}

void DescriptorExtractor::calcOrientationHistogram(Descriptor& descriptor, const DoubleMatrix& dirs, const DoubleMatrix& grad, KeyPoint& point)
{
	int bins = descriptor.getBinCount();
	int radius = gridSize / 2;
	double binSize = 2 * M_PI / bins;
	DoubleMatrix gauss = DoubleMatrix::createGaussian(gridSize + 1, gridSize + 1, 1.5);

	for (int i = -radius; i < radius; i++) {
		for (int j = -radius; j < radius; j++) {
			double dist = sqrt(i * i + j * j);
			double phi = dirs.get(point.y + i, point.x + j);

			std::pair<int, int> binsIndex = getBinsIndexies(phi, binSize, bins);
			double bin1Center = binsIndex.first * binSize + binSize / 2;
			double distToBin1Center = abs(bin1Center - phi);
			double distToBin2Center = binSize - distToBin1Center;
			int ii = i + radius;
			int jj = j + radius;
			double gradVal = grad.get(point.y + i, point.x + j);
			descriptor.at(0, binsIndex.first) += gradVal * (1 - distToBin1Center / binSize) * gauss.at(ii, jj);
			descriptor.at(0, binsIndex.second) += gradVal * (1 - distToBin2Center / binSize) * gauss.at(ii, jj);
		}
	}
}

std::vector<KeyPoint> DescriptorExtractor::calcPointsOrientation(const DoubleMatrix& img, std::vector<KeyPoint>& points, int bins)
{
	DoubleMatrix gradient = img.calcSobel();
	DoubleMatrix gradientDirs = img.gradientDirection();
	std::vector<Descriptor> descriptors;
	int radius = gridSize / 2;

	std::vector<KeyPoint> result;
	for (int ip = 0; ip < points.size(); ip++) {
		descriptors.push_back(Descriptor(1, bins));
		calcOrientationHistogram(descriptors[ip], gradientDirs, gradient, points[ip]);
		//descriptors[ip].vals().sort([&](double a, double b) { return a > b; });

		int count = descriptors[ip].vals().getSize();
		int maxIndex = -1;
		int secondMaxIndex = -1;
		double maxEl = std::numeric_limits<double>::min();
		double secondMaxEl = std::numeric_limits<double>::min();
		for (int i = 0; i < count; i++) {
			double v = descriptors[ip].vals()[i];
			if (v > maxEl) {
				secondMaxEl = maxEl;
				secondMaxIndex = maxIndex;
				maxEl = v;
				maxIndex = i;
			}
			else if (v > secondMaxEl) {
				secondMaxEl = v;
				secondMaxIndex = i;
			}
		}

		KeyPoint p1(points[ip]);
		p1.angle = maxIndex * (2 * M_PI / bins);
		result.push_back(p1);
		if (secondMaxEl >= 0.8 * maxEl) {
			KeyPoint p2(points[ip]);
			p2.angle = secondMaxIndex * (2 * M_PI / bins);
			result.push_back(p2);
		}
	}

	return result;
}

std::vector<std::pair<int, int>> DescriptorExtractor::findMatches(std::vector<Descriptor> aDescriptors, std::vector<Descriptor> bDescriptors, double threshold)
{
	// Вектор расстояний дескрипторов изображения A до дескрипторов B и соответствующих индексов B
	std::vector<std::vector<std::pair<double, int>>> distances(aDescriptors.size());
	for (auto& row : distances) {
		row.resize(bDescriptors.size());
	}

	for (int i = 0; i < aDescriptors.size(); i++) {
		for (int j = 0; j < bDescriptors.size(); j++) {
			distances[i][j].first = Descriptor::distance(aDescriptors[i], bDescriptors[j]);
			distances[i][j].second = j;
		}
	}

	// Сортировка по минимальному расстоянию
	for (auto& vec : distances) {
		std::sort(begin(vec), end(vec),
			[&](std::pair<double, int>& a, std::pair<double, int>& b)
			{ return a.first < b.first; });
	}

	std::vector<std::pair<int, int>> result;
	// Определение совпадений с помощью NNDR
	for (int i = 0; i < aDescriptors.size(); i++) {
		double minDist = distances[i][0].first;
		double secondMinDist = distances[i][1].first;
		if (minDist / secondMinDist < threshold) {
			result.push_back(std::make_pair(i, distances[i][0].second));
		}
	}

	return result;
}
