#include <QtCore/QCoreApplication>
#include <QtGui>

#include <iostream>
#include <chrono>

#include "LabImage.h"
#include "IntMatrix.h"
#include "DoubleMatrix.h"

using timeCl = std::chrono::high_resolution_clock;
using timeMS = std::chrono::milliseconds;

double parseDoubleOrDefault(const QString& line, double dflt) {
	bool* isDouble = new bool;
	double val = line.toDouble(isDouble);
	double result = *isDouble ? val : dflt;
	delete isDouble;
	return result;
}

int parseIntOrDefault(const QString line, int dflt) {
	bool* isInt = new bool;
	int val = line.toInt(isInt);
	int result = *isInt ? val : dflt;
	delete isInt;
	return result;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QCommandLineParser parser;
	QCommandLineOption borderOption("border", "Image border type (0 - black, 1 - border pixel, 2 - reflect (default), 3 - wrap )", "borderType", "2");
	QCommandLineOption sigmaOption("sigma", "Sigma value for gauss filter (default = 1.0)", "sigmaVal", "1.0");
	parser.addHelpOption();
	parser.addPositionalArgument("source", "Source Image ");
	parser.addOption(borderOption);
	parser.addOption(sigmaOption);

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
	std::cout << "File: " << sourceFileInfo.fileName().toStdString() << " ";
	QImage mainImage(sourceFileInfo.absoluteFilePath());
	LabImage l(mainImage);
	l.printInfo();
	IntMatrix img = IntMatrix::fromImage(l.getGrayScale());
	DoubleMatrix doubleImg = img.toDoubleMatrix();

	doubleImg.normalize(0, 1);
	DoubleMatrix dx = doubleImg.dx();
	DoubleMatrix dy = doubleImg.dy();
	DoubleMatrix sobel = doubleImg.calcSobel();
	DoubleMatrix gauss = doubleImg.gaussian(sigma);

	LabImage::saveImage(dx.normalize(0,255), "out-dx.png");
	LabImage::saveImage(dy.normalize(0, 255) , "out-dy.png");
	LabImage::saveImage(sobel.normalize(0, 255), "out-sobel.png");
	LabImage::saveImage(gauss.normalize(0, 255), "out-gauss.png");

	auto end = timeCl::now();
	auto delta = std::chrono::duration_cast<timeMS>(end - start);
	std::cout << "Image Processing Complete(" << delta.count() << "ms)" << std::endl;

	return 0;
}
