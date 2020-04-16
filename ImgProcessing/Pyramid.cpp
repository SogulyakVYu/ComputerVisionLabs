#include "Pyramid.h"
#include <iostream>

#include "LabImage.h"

std::vector<PyramidRow>& Pyramid::get()
{
	return pyramid;
}

PyramidRow& Pyramid::get(int i)
{
	return (*this)[i];
}

DoubleMatrix& Pyramid::getImage(int i)
{
	return pyramid[i].image;
}

void Pyramid::saveImage(const QString& dir)
{
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

Pyramid Pyramid::createFrom(DoubleMatrix& image, double sigmaA, double sigma0, int octaveCount, int levelCount)
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
