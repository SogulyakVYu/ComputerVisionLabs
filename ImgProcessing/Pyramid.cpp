#include "Pyramid.h"
#include <iostream>

#include "LabImage.h"

std::vector<double> Pyramid::getNeighbors3d(int x, int y, int iOctave, int iLevel, int winSize)
{
	std::vector<double> neighbors;
	PyramidRow& prev = get(iOctave, iLevel - 1);
	PyramidRow& cur = get(iOctave, iLevel);
	PyramidRow& next = get(iOctave, iLevel + 1);
	int offset = winSize / 2;
	for (int u = -offset; u <= offset; u++) {
		for (int v = -offset; v <= offset; v++) {
			neighbors.push_back(prev.image.get(u + y, v + x));
			if (u != 0 || v != 0) {
				neighbors.push_back(cur.image.get(u + y, v + x));
			}
			neighbors.push_back(next.image.get(u + y, v + x));
		}
	}

	return neighbors;
}

std::vector<PyramidRow>& Pyramid::get()
{
	return pyramid;
}

PyramidRow& Pyramid::get(int i)
{
	return (*this)[i];
}

PyramidRow& Pyramid::getBySigma(double sigma)
{
	// Общее число изображений в пирамиде
	int imageCount = levelCount * octaveCount;
	// Ближайший к заданной сигме номер в геометрической прогрессии
	double sigmaIndex = (std::log(sigma / sigma0) / std::log(sigmaStep));
	sigmaIndex = std::round(sigmaIndex);
	sigmaIndex = sigmaIndex < 0 ? 0 : sigmaIndex >= imageCount ? imageCount - 1 : sigmaIndex;
	// Найденная строка
	PyramidRow& r = pyramid[rowsBySigma[(int)sigmaIndex]];
	//qDebug() << "getFromSigma: sigma:" << sigma << "index:" << sigmaIndex << "(" << r.octave << "," << r.level << ")=" << r.sigmaEffective;
	return  pyramid[rowsBySigma[(int)sigmaIndex]];
}

PyramidRow& Pyramid::getBySigma(int octave, double sigma)
{
	auto closest = std::min_element(begin(pyramid), end(pyramid),
		[&](PyramidRow& row, PyramidRow& min)
		{ 
			return row.octave == octave && abs(sigma - row.sigmaEffective) < abs(sigma - min.sigmaEffective);
		});
	return *closest;
}

DoubleMatrix& Pyramid::getImage(int i)
{
	return pyramid[i].image;
}

void Pyramid::saveImage(const QString& dir, int nameFormat)
{
	QDir saveDir(dir);
	if (!saveDir.exists()) QDir().mkdir(dir);

	if (nameFormat == 0) {
		for (PyramidRow& row : pyramid) {
			QString fileName = "\\oct["
				+ QString::number(row.octave) + ","
				+ QString::number(row.level) + "] "
				+ QString::number(row.sigmaLocal) + " "
				+ QString::number(row.sigmaEffective)
				+ ".jpg";
			LabImage::saveImage(row.image, dir + fileName);
		}
	}
	else if (nameFormat == 1) {
		for (PyramidRow& row : pyramid) {
			QString fileName = "\\oct["
				+ QString::number(row.octave) + ","
				+ QString::number(row.level) + "]"
				+ ".jpg";
			LabImage::saveImage(row.image, dir + fileName);
		}
	}
	
}

double Pyramid::getPixel(int x, int y, double sigma)
{
	int imageCount = levelCount * octaveCount;
	double imageIndex = (std::log(sigma / sigma0) / std::log(sigmaStep));
	imageIndex = std::round(imageIndex + imageIndex / levelCount);
	imageIndex = imageIndex < 0 ? 0 : imageIndex >= imageCount? imageCount - 1 : imageIndex;

	PyramidRow foundRow = pyramid[(int)imageIndex];
	int newY = (y / std::pow(2, foundRow.octave));
	int newX = (x / std::pow(2, foundRow.octave));
	std::cout << "Octave: " << foundRow.octave << " Level: " << foundRow.level;
	std::cout << " Local=" << foundRow.sigmaLocal << " Effective=" << foundRow.sigmaEffective << " L=" << foundRow.image.at(newY, newX) << std::endl;
	return foundRow.image.at(newY, newX);
}

Pyramid Pyramid::createDoGPyramid()
{
	Pyramid result;
	result.octaveCount = octaveCount;
	result.levelCount = levelCount - 1;
	result.sigma0 = sigma0;
	result.sigmaStep = sigmaStep;

	for (int i = 0; i < octaveCount; i++) {
		for (int j = 1; j < levelCount; j++) {
			PyramidRow first = get(i, j - 1);
			PyramidRow second = get(i, j);
			DoubleMatrix diff = second.image.sub(first.image);
			result.pyramid.push_back({i, j - 1, first.sigmaLocal, first.sigmaEffective, diff});
		}
	}

	return result;
}

Pyramid Pyramid::createHarrisPyramid(int windowSize)
{
	Pyramid harris;
	harris.octaveCount = octaveCount;
	harris.levelCount = levelCount;
	harris.overlapCount = overlapCount;
	harris.sigma0 = sigma0;
	harris.sigmaStep = sigmaStep;
	for (PyramidRow& row : pyramid) {
		harris.pyramid.push_back({row.octave, row.level, row.sigmaLocal, row.sigmaEffective, row.image.operatorHarris(windowSize)});
	}
	return harris;
}

Pyramid Pyramid::createGradientPyramid()
{
	Pyramid gradients;
	gradients.octaveCount = octaveCount;
	gradients.levelCount = levelCount;
	gradients.overlapCount = overlapCount;
	gradients.sigma0 = sigma0;
	gradients.sigmaStep = sigmaStep;

	for (PyramidRow& row : pyramid) {
		gradients.pyramid.push_back({ row.octave, row.level, row.sigmaLocal, row.sigmaEffective, row.image.calcSobel() });
	}

	return gradients;
}

Pyramid Pyramid::createDirectionsPyramid()
{
	Pyramid directions;
	directions.octaveCount = octaveCount;
	directions.levelCount = levelCount;
	directions.overlapCount = overlapCount;
	directions.sigma0 = sigma0;
	directions.sigmaStep = sigmaStep;

	for (PyramidRow& row : pyramid) {
		directions.pyramid.push_back({ row.octave, row.level, row.sigmaLocal, row.sigmaEffective, row.image.gradientDirection() });
	}

	return directions;
}

std::vector<KeyPoint> Pyramid::findExtremePoints(int winSize, double threshold)
{
	std::vector<KeyPoint> points;

	for (int iOct = 0; iOct < octaveCount; iOct++) {
		for (int iLevel = 1; iLevel < levelCount - 1; iLevel++) {
			PyramidRow& prev = get(iOct, iLevel - 1);
			PyramidRow& cur = get(iOct, iLevel);
			PyramidRow& next = get(iOct, iLevel + 1);

			int width = cur.image.getWidth();
			int height = cur.image.getHeight();
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					double extremum = cur.image.at(y, x);
					std::vector<double> neighbors = getNeighbors3d(x, y, iOct, iLevel, winSize);
					auto minmax_el = std::minmax_element(begin(neighbors), end(neighbors));
					double minEl = *minmax_el.first;
					double maxEl = *minmax_el.second;
					if ((extremum < minEl || extremum > maxEl) && abs(extremum) > threshold ) {
						KeyPoint pt(x, y, extremum);
						pt.sigma = cur.sigmaEffective;
						points.push_back(pt);
						//qDebug() << "oct[" << iOct << "," << iLevel << "] p=" << extremum << " s=" << pt.sigma;
					}
				}
			}
		}
	}

	return points;
}

Pyramid Pyramid::createFrom(const DoubleMatrix& image, double sigmaA, double sigma0, int octaveCount, int levelCount)
{
	double firstSigma = std::sqrt((sigma0 * sigma0) - (sigmaA * sigmaA));
	double sigma = abs(firstSigma) < 0.0001 ? 1 : firstSigma;
	double levelStep = std::pow(2, 1.0 / (levelCount - 1));

	Pyramid result;
	result.octaveCount = octaveCount;
	result.levelCount = levelCount;
	result.sigma0 = sigma0;
	result.sigmaStep = levelStep;
	double summarySigma = sigma0;

	DoubleMatrix f(image);
	f = f.gaussian(sigma);
	for (int iOctave = 0; iOctave < octaveCount; iOctave++) {
		sigma = sigma0;
		result.pyramid.push_back({ iOctave, 0, sigma, summarySigma, f});
		for (int iLevel = 1; iLevel < levelCount; iLevel++) {
			double newSigma = sigma * levelStep;
			double sigmaTo = std::sqrt(newSigma * newSigma - sigma * sigma);
			f = f.gaussian(sigmaTo);
			sigma = newSigma;
			summarySigma *= levelStep;
			result.pyramid.push_back({iOctave, iLevel, sigma, summarySigma,  f});

		}
		f = f.downsample();
	}

	return result;
}

Pyramid Pyramid::createWithOverlap(const DoubleMatrix& image, double sigmaA, double sigma0, int octaveCount, int levelCount, int overlap)
{
	double firstSigma = std::sqrt((sigma0 * sigma0) - (sigmaA * sigmaA));
	double sigma = abs(firstSigma) < 0.0001 ? 1 : firstSigma;
	double levelStep = std::pow(2, 1.0 / (levelCount - 1));

	Pyramid result;
	result.octaveCount = octaveCount;
	result.levelCount = levelCount + overlap;
	result.overlapCount = overlap;
	result.sigma0 = sigma0;
	result.sigmaStep = levelStep;
	double summarySigma = sigma0;
	DoubleMatrix curImg(image);
	curImg = curImg.gaussian(sigma);

	result.rowsBySigma.push_back(0);
	for (int iOctave = 0; iOctave < octaveCount; iOctave++) {
		sigma = sigma0;
		
		// Основные изображения октавы
		result.pyramid.push_back({ iOctave, 0, sigma, summarySigma, curImg }); 
		for (int iLevel = 1; iLevel < levelCount; iLevel++) {
			double newSigma = sigma * levelStep;
			double sigmaTo = std::sqrt(newSigma * newSigma - sigma * sigma);
			curImg = curImg.gaussian(sigmaTo);
			sigma = newSigma;
			summarySigma *= levelStep;
			result.pyramid.push_back({ iOctave, iLevel, sigma, summarySigma,  curImg });
			result.rowsBySigma.push_back(iOctave * result.levelCount + iLevel);
		}
		// Дополнительные изображения для построения DoG
		DoubleMatrix overlapImg(curImg);
		double overlapSigma = sigma;
		double overlapSumSigma = summarySigma;
		for (int i = 0; i < overlap; i++) {
			double newSigma = overlapSigma * levelStep;
			double sigmaTo = std::sqrt(newSigma * newSigma - overlapSigma * overlapSigma);
			overlapImg = overlapImg.gaussian(sigmaTo);
			overlapSigma = newSigma;
			overlapSumSigma *= levelStep;
			result.pyramid.push_back({iOctave, levelCount + i, overlapSigma, overlapSumSigma, overlapImg});
		}
		curImg = curImg.downsample();
	}

	return result;
}
