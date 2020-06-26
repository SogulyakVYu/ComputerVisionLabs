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

std::vector<std::pair<int, double>> DescriptorExtractor::getHistogramVals(Descriptor& d, double x, double y) 
{
	int gridSize = d.getGridSize();
	double radius = gridSize / 2.;
	int cellCount = d.getCellCount();
	double cellSize = d.getCellSize();
	int i = std::floor(y + radius);
	int j = std::floor(x + radius);
	double histRadius = cellSize / 2;
	// Номер [x,y] гистограммы, в которой находится точка
	int yHist = i / cellSize;
	int xHist = j / cellSize;
	// координаты центра гистограммы
	double histCenterX = xHist * cellSize + histRadius;
	double histCenterY = yHist * cellSize + histRadius;

	// Смещение смежных гистограмм
	int yHistOffset, xHistOffset;
	if (y + radius < histCenterY) yHistOffset = -1;
	else  yHistOffset = 1;
	if (x + radius < histCenterX) xHistOffset = -1;
	else xHistOffset = 1;
	// Номера смежных гистограмм
	std::vector<int> xhistIndexes = { xHist, xHist + xHistOffset };
	std::vector<int> yhistIndexes = { yHist, yHist + yHistOffset };
	double distClosestCenter = dist(x + radius, y + radius, histCenterX, histCenterY);
	// Номера и веса для смежных гистограмм
	std::vector<std::pair<int, double>> resultHistVals;
	double r = sqrt(2 * cellSize * cellSize);
	for (int yHistIndex : yhistIndexes) {
		for (int xHistIndex : xhistIndexes) {
			double xC = xHistIndex * cellSize + histRadius;
			double yC = yHistIndex * cellSize + histRadius;
			double distX = 1 - abs(x + radius - xC) / cellSize;
			double distY = 1 - abs(y + radius - yC) / cellSize;
			int curHistogram = (int)yHistIndex * cellCount + (int)xHistIndex;
			if (xHistIndex >= 0 && xHistIndex < cellCount && yHistIndex >= 0 && yHistIndex < cellCount) {
				resultHistVals.push_back({ curHistogram, distX * distY});
			}
		}
	}

	return resultHistVals;
}

DescriptorExtractor::DescriptorExtractor(int gridSize, int cellCount, int histogramCount, int binCount):
	extractorGridSize(gridSize), extractorCellCount(cellCount), extractorCellSize(gridSize/cellCount), extractorHistogramCount(histogramCount), extractorBinCount(binCount) {}

DescriptorExtractor::DescriptorExtractor(int gridSize, int cellCount, int binCount) :
	extractorGridSize(gridSize), extractorCellCount(cellCount), extractorCellSize(gridSize / cellCount), extractorHistogramCount(cellCount * cellCount), extractorBinCount(binCount) {}

void DescriptorExtractor::fillDescriptor(Descriptor& descriptor, const DoubleMatrix& dirs, const DoubleMatrix& grad, KeyPoint& point)
{
	int radius = extractorGridSize / 2;
	double binSize = 2 * M_PI / extractorBinCount;
	DoubleMatrix gauss = DoubleMatrix::createGaussian(extractorGridSize + 1, extractorGridSize + 1, extractorGridSize / 6.);

	for (int i = -radius; i < radius; i++) {
		for (int j = -radius; j < radius; j++) {
			double phi = dirs.get(point.y + i, point.x + j);

			std::pair<int, int> binsIndex = getBinsIndexies(phi, binSize, extractorBinCount);
			double bin1Center = binsIndex.first * binSize + binSize / 2;
			double distToBin1Center = abs(bin1Center - phi);
			double distToBin2Center = binSize - distToBin1Center;
			int ii = i + radius;
			int jj = j + radius;
			int curHistogram = (ii / extractorCellSize) * extractorCellCount + (jj / extractorCellSize);
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
		descriptors.push_back(Descriptor(extractorGridSize, extractorCellCount, extractorBinCount));
		fillDescriptorAngle(descriptors[i], gradientDirs, gradient, points[i]);
		descriptors[i].normalize();
		descriptors[i].truncate(0.2);
		descriptors[i].normalize();
	}

	return descriptors;
}

void DescriptorExtractor::fillDescriptorAngle(Descriptor& descriptor, const DoubleMatrix& dirs, const DoubleMatrix& grad, KeyPoint& point)
{
	int gridSize = descriptor.getGridSize();
	int cellSize = descriptor.getCellSize();
	int cellCount = descriptor.getCellCount();
	int binCount = descriptor.getBinCount();
	int radius = gridSize / 2;
	double binSize = 2 * M_PI / binCount;
	double twoPi = 2 * M_PI;
	DoubleMatrix gauss = DoubleMatrix::createGaussian(gridSize + 1, gridSize + 1, gridSize / 6.);

	for (int y = -radius; y < radius; y++) {
		for (int x = -radius; x < radius; x++) {
			double x1 = x * cos(point.angle) - y * sin(point.angle);
			double y1 = y * cos(point.angle) + x * sin(point.angle);
			x1 = std::round(x1);
			y1 = std::round(y1);

			if (y == -radius || x == -radius || y == radius - 1 || x == radius - 1) {
				gridPoints.push_back(KeyPoint(point.x + x1, point.y + y1, 0, point.angle));
			}
			
			double phi = dirs.get(point.y + y1, point.x + x1);
			phi = phi - point.angle;
			phi = phi < 0 ? phi + twoPi : phi;
			phi = phi > twoPi ? phi - twoPi : phi;

			std::pair<int, int> binsIndex = getBinsIndexies(phi, binSize, binCount);
			double bin1Center = binsIndex.first * binSize + binSize / 2;
			double distToBin1Center = abs(bin1Center - phi);
			double distToBin2Center = binSize - distToBin1Center;
			int iHist = y + radius;
			int jHist = x + radius;
			int curHistogram = (iHist / cellSize) * cellCount + (jHist / cellSize);
			double gradVal = grad.get(point.y + y1, point.x + x1);
			descriptor.at(curHistogram, binsIndex.first) += gradVal * (1 - distToBin1Center / binSize) * gauss.at(iHist, jHist);
			descriptor.at(curHistogram, binsIndex.second) += gradVal * (1 - distToBin2Center / binSize) * gauss.at(iHist, jHist);
		}
	}
}

void DescriptorExtractor::fillDescriptorScale(Descriptor& descriptor, const DoubleMatrix& dirs, const DoubleMatrix& grad, KeyPoint& point)
{
	int gridSize = descriptor.getGridSize();
	double cellSize = descriptor.getCellSize();
	int cellCount = descriptor.getCellCount();
	int binCount = descriptor.getBinCount();
	double radius = gridSize / 2.;
	double binSize = 2 * M_PI / binCount;
	double twoPi = 2 * M_PI;
	int gaussSize = gridSize % 2 == 0 ? gridSize + 1 : gridSize;
	DoubleMatrix gauss = DoubleMatrix::createGaussian(gaussSize, gaussSize, 0.5 * gridSize);

	for (double y = -radius; y < radius; y++) {
		for (double x = -radius; x < radius; x++) {
			double x1 = x * cos(point.angle) - y * sin(point.angle);
			double y1 = y * cos(point.angle) + x * sin(point.angle);
			x1 = std::round(x1);
			y1 = std::round(y1);

			double phi = dirs.get(point.y + y1, point.x + x1);
			phi = phi - point.angle;
			phi = phi < 0 ? phi + twoPi : phi;
			phi = phi > twoPi ? phi - twoPi : phi;

			std::pair<int, int> binsIndex = getBinsIndexies(phi, binSize, binCount);
			double bin1Center = binsIndex.first * binSize + binSize / 2;
			double distToBin1Center = abs(bin1Center - phi);
			double distToBin2Center = binSize - distToBin1Center;
			int i = std::floor(y + radius);
			int j = std::floor(x + radius);
			std::vector<std::pair<int, double>> resultHistVals = getHistogramVals(descriptor, x, y);

			double gradVal = grad.get(point.y + y1, point.x + x1);
			for (auto& val : resultHistVals) {
				int curHistogram = val.first;
				double w = val.second;
				descriptor.at(curHistogram, binsIndex.first) += gradVal * (1 - distToBin1Center / binSize) * w * gauss.at(i, j);
				descriptor.at(curHistogram, binsIndex.second) += gradVal * (1 - distToBin2Center / binSize) * w * gauss.at(i, j);
			}
		}
	}
}

void DescriptorExtractor::calcOrientationHistogram(Descriptor& descriptor, const DoubleMatrix& dirs, const DoubleMatrix& grad, KeyPoint& point)
{
	int bins = descriptor.getBinCount();
	int gridSize = descriptor.getGridSize();
	double radius = gridSize / 2.;
	double binSize = 2 * M_PI / bins;
	int gaussSize = gridSize % 2 == 0 ? gridSize + 1 : gridSize;
	DoubleMatrix gauss = DoubleMatrix::createGaussian(gaussSize, gaussSize, 1.5 * (point.sigma == 0.0 ? 1 : point.sigma));

	for (double i = -radius; i < radius; i++) {
		for (double j = -radius; j < radius; j++) {
			double phi = dirs.get(point.y + i, point.x + j);

			std::pair<int, int> binsIndex = getBinsIndexies(phi, binSize, bins);
			double bin1Center = binsIndex.first * binSize + binSize / 2;
			double distToBin1Center = abs(bin1Center - phi);
			double distToBin2Center = binSize - distToBin1Center;
			int ii = std::round(i + radius);
			int jj = std::round(j + radius);
			double gradVal = grad.get(point.y + i, point.x + j);
			descriptor.at(0, binsIndex.first) += gradVal * (1 - distToBin1Center / binSize) * gauss.at(ii, jj);
			descriptor.at(0, binsIndex.second) += gradVal * (1 - distToBin2Center / binSize) * gauss.at(ii, jj);
		}
	}
}

void DescriptorExtractor::addPointWithPeaks(KeyPoint& point, Descriptor& descriptor, std::vector<KeyPoint>& out, int bins) 
{
	int count = descriptor.vals().getSize();
	int maxIndex = -1;
	int secondMaxIndex = -1;
	double maxEl = std::numeric_limits<double>::min();
	double secondMaxEl = std::numeric_limits<double>::min();
	for (int i = 0; i < count; i++) {
		double v = descriptor.vals()[i];
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

	KeyPoint p1(point);
	p1.angle = maxIndex * (2 * M_PI / bins);
	out.push_back(p1);
	if (secondMaxEl >= 0.8 * maxEl) {
		KeyPoint p2(point);
		p2.angle = secondMaxIndex * (2 * M_PI / bins);
		out.push_back(p2);
	}
}

std::vector<KeyPoint> DescriptorExtractor::calcPointsOrientation(const DoubleMatrix& img, std::vector<KeyPoint>& points, int bins)
{
	DoubleMatrix gradient = img.calcSobel();
	DoubleMatrix gradientDirs = img.gradientDirection();
	std::vector<Descriptor> descriptors;
	int gridSize = 16;
	int radius = gridSize / 2;

	std::vector<KeyPoint> result;
	for (int ip = 0; ip < points.size(); ip++) {
		descriptors.push_back(Descriptor(gridSize, 1, bins));
		calcOrientationHistogram(descriptors[ip], gradientDirs, gradient, points[ip]);
		addPointWithPeaks(points[ip], descriptors[ip], result, bins);
	}

	return result;
}

std::pair<std::vector<KeyPoint>, std::vector<Descriptor>>  DescriptorExtractor::computeScale(Pyramid& pyramid, std::vector<KeyPoint>& points)
{
	std::vector<KeyPoint> orientPoints;
	std::vector<Descriptor> descriptors;
	std::vector<KeyPoint> resultPoints;

	int cellCount = 4;
	int binCount = 8;
	int octaveCount = pyramid.getOctaveCount();
	int levelCount = pyramid.getLevelCount();
	int overlap = pyramid.getOverlapCount();
	Pyramid gradients = pyramid.createGradientPyramid();
	Pyramid directions = pyramid.createDirectionsPyramid();

	int bins = 36; 
	// Определение ориентации точки
	for (int iOctave = 0; iOctave < octaveCount; iOctave++) {
		double firstSigma = pyramid.get(iOctave, 0).sigmaEffective;
		double lastSigma = pyramid.get(iOctave, levelCount - overlap - 1).sigmaEffective;
		for (KeyPoint& point : points) {
			if (firstSigma < point.sigma && point.sigma <= lastSigma) {
				int gridSize = std::round(16 * point.sigma / firstSigma);
				Descriptor d(gridSize, 1, bins);
				PyramidRow& dirs = directions.getBySigma(iOctave, point.sigma);
				PyramidRow& grads = gradients.getBySigma(iOctave, point.sigma);
				calcOrientationHistogram(d, dirs.image, grads.image, point);
				addPointWithPeaks(point, d, orientPoints, bins);
			}
		}
	}

	// Заполнение дескрипторов
	for (int iOctave = 0; iOctave < octaveCount; iOctave++) {
		double firstSigma = pyramid.get(iOctave, 0).sigmaEffective;
		double lastSigma = pyramid.get(iOctave, levelCount - overlap - 1).sigmaEffective;
		for (KeyPoint& point : orientPoints) {
			if (firstSigma < point.sigma && point.sigma <= lastSigma) {
				int gridSize = std::round(16 * point.sigma / firstSigma);
				Descriptor d(gridSize, cellCount, binCount);
				PyramidRow& dirs = directions.getBySigma(iOctave, point.sigma);
				PyramidRow& grads = gradients.getBySigma(iOctave, point.sigma);
				fillDescriptorScale(d, dirs.image, grads.image, point);
				d.normalize();
				d.truncate(0.2);
				d.normalize();
				descriptors.push_back(d);
				// Местоположение точки на изначальном изображении
				KeyPoint scalePoint(point);
				int scale = std::pow(2, iOctave);
				scalePoint.x *= scale;
				scalePoint.y *= scale;
				resultPoints.push_back(scalePoint);
			}
		}
	}
	qDebug() << "Proccessed points:" << resultPoints.size();

	return std::make_pair(resultPoints, descriptors);
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

	qDebug() << "Matches found: " << result.size();

	return result;
}
