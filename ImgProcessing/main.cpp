#include <QtCore/QCoreApplication>
#include <QtGui>

#include <iostream>
#include <chrono>
#include <cassert>

#include "LabImage.h"
#include "IntMatrix.h"
#include "DoubleMatrix.h"
#include "Pyramid.h"

using timeCl = std::chrono::high_resolution_clock;
using timeMS = std::chrono::milliseconds;

double parseDoubleOrDefault(const QString& line, double dflt) {
	bool isDouble = false;
	double val = line.toDouble(&isDouble);
	double result = isDouble ? val : dflt;
	return result;
}

int parseIntOrDefault(const QString& line, int dflt) {
	bool isInt = false;
	int val = line.toInt(&isInt);
	int result = isInt ? val : dflt;
	return result;
}

std::vector<double> parseDoubleVector(const QString& line, const QString& delim) {
	std::vector<double> result(0);
	QStringList words = line.split(delim);
	for (QString& word : words) {
		bool isDouble = false;
		double val = word.toDouble(&isDouble);
		if (isDouble) result.push_back(val);
	}

	return result;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QCommandLineParser parser;
	parser.addHelpOption();
	parser.addPositionalArgument("source", "Source Image ");
	QCommandLineOption borderOption("border", "Image border type (0 - black, 1 - border pixel, 2 - reflect (default), 3 - wrap )", "borderType", "2");
	parser.addOption(borderOption);
	QCommandLineOption sigmaOption("sigma", "Sigma value for gauss filter (default = 1.0)", "sigmaVal", "1.0");
	parser.addOption(sigmaOption);
	QCommandLineOption pyramidOption("pyramid", "Create pyramid from 'sigmaA;sigma0;octaveCount;levelCount'", "pyramidVal");
	parser.addOption(pyramidOption);
	QCommandLineOption lOption("L", "L=x;y;sigma", "lVal");
	parser.addOption(lOption);
	QCommandLineOption showInfoOption("info", "Shows file size and image processing time");
	parser.addOption(showInfoOption);
	QCommandLineOption lab1Option("Lab1", "Create dx,dy, gauss, sobel image");
	parser.addOption(lab1Option);
	parser.process(a);

	const QStringList posArgs = parser.positionalArguments();
	QFileInfo sourceFileInfo(posArgs[0]);
	int borderInt = parseIntOrDefault(parser.value(borderOption), 2);
	DoubleMatrix::BorderType borderType = borderInt < 5 && borderInt >= 0 ? static_cast<DoubleMatrix::BorderType>(borderInt) : DoubleMatrix::BorderType::Reflect;
	DoubleMatrix::setDefaultBoderType(borderType);
	double sigma = parseDoubleOrDefault(parser.value(sigmaOption), 1.0);

	if (!sourceFileInfo.exists()) {
		std::cout << sourceFileInfo.fileName().toStdString() << " - file doesn't exist";
		return 0;
	}

	auto start = timeCl::now();
	QImage mainImage(sourceFileInfo.absoluteFilePath());
	LabImage l(mainImage);

	if (parser.isSet(showInfoOption)) {
		std::cout << "File: " << sourceFileInfo.fileName().toStdString() << " ";
		l.printInfo();
	}

	IntMatrix img = IntMatrix::fromImage(l.getGrayScale());
	DoubleMatrix doubleImg = img.toDoubleMatrix();
	doubleImg.normalize(0, 1);

	if (parser.isSet(lab1Option)) {
		DoubleMatrix dx = doubleImg.dx();
		DoubleMatrix dy = doubleImg.dy();
		DoubleMatrix sobel = doubleImg.calcSobel();
		DoubleMatrix gauss = doubleImg.gaussian(sigma);

		LabImage::saveImage(dx, "out-dx.png");
		LabImage::saveImage(dy, "out-dy.png");
		LabImage::saveImage(sobel, "out-sobel.png");
		LabImage::saveImage(gauss, "out-gauss.png");
	}

	if (parser.isSet(pyramidOption)) {
		std::vector<double> pyramidVals = parseDoubleVector(parser.value(pyramidOption), ";");
		if (pyramidVals.size() == 4) {
			auto pyramid = Pyramid::createFrom(doubleImg, pyramidVals[0], pyramidVals[1], pyramidVals[2], pyramidVals[3]);
			pyramid.saveImage(a.applicationDirPath() + "\\pyramid");
			if (parser.isSet(lOption)) {
				std::vector<double> lVals = parseDoubleVector(parser.value(lOption), ";");
				if (lVals.size() == 3) {
						double l = pyramid.getPixel(lVals[0], lVals[1], lVals[2]);
						std::cout << "L(" << lVals[0] << "," << lVals[1] << "," << lVals[2] << ")=" << l << std::endl;
				}
				else {
					std::cout << "--L arguments is incorrect: " << parser.value(lOption).toStdString() << std::endl;
				}
			}
		}
		else {
			std::cout << "--pyramid arguments is incorrect: " << parser.value(pyramidOption).toStdString() << std::endl;
		}
	}

	auto end = timeCl::now();
	auto delta = std::chrono::duration_cast<timeMS>(end - start);
	if (parser.isSet(showInfoOption)) {
		std::cout << "Image Processing Complete(" << delta.count() << "ms)" << std::endl;
	}

	return 0;
}
