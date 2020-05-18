#include <QtCore/qmath.h>
#include <QtCore/qdebug.h>
#include "DescriptorExtractor.h"

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

std::vector<Descriptor> DescriptorExtractor::compute(const DoubleMatrix& img, std::vector<KeyPoint>& points)
{
	DoubleMatrix gradient = img.calcSobel();
	DoubleMatrix gradientDirs = img.gradientDirection();
	std::vector<Descriptor> descriptors;
	for (int i = 0; i < points.size(); i++) {
		descriptors.push_back(Descriptor(gridSize, cellCount, binCount));
		fillDescriptor(descriptors[i], gradientDirs, gradient, points[i]);
		descriptors[i].normalize();
		descriptors[i].truncate(0.2);
		descriptors[i].normalize();
	}

	return descriptors;
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
