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

void processLab1Option(const QCommandLineParser& parser, QList<QCommandLineOption>& options, DoubleMatrix& source);
void processPyramidOption(const QCommandLineParser& parser, QList<QCommandLineOption>& options, DoubleMatrix& source, QCoreApplication& a);
void processMoravecAndHarrisOption(QString& srcName,const QCommandLineParser& parser, QList<QCommandLineOption>& options, DoubleMatrix& source);
void callCornerDetectorMethod(QString& srcName, char method, DoubleMatrix& src, DoubleMatrix& img, std::vector<double> params, bool withANMS, int pointCount);

template<typename T>
void printValues(const std::vector<std::string>& text, const std::vector<T>& values) {
	
	int len = values.size();
	std::cout << text[0] << ": " << values[0];
	for (int i = 1; i < len; i++) {
		std::cout << ", " << text[i] << ": " << values[i];
	}
	std::cout << std::endl;
}

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
	QCommandLineOption moravicDetectorOption("moravec", "Moravec corner detector 'winSize;localMaxWinSize;threshold", "moravecVal");
	parser.addOption(moravicDetectorOption);
	QCommandLineOption harrisDetectorOption("harris", "Harris corner detector 'winSize;localMaxWinSize;threshold", "harrisVal");
	parser.addOption(harrisDetectorOption);
	QCommandLineOption anmsOption("anms", "ANMS filter ", "anmsVal");
	parser.addOption(anmsOption);
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

	// Обработка изображения
	IntMatrix img = IntMatrix::fromImage(l.getGrayScale());
	DoubleMatrix doubleImg = img.toDoubleMatrix();
	doubleImg.norm1();

	// Если задан флаг --Lab1
	processLab1Option(parser, QList<QCommandLineOption>{ lab1Option, sigmaOption }, doubleImg);
	// Если задан аргумент --pyramid
	processPyramidOption(parser, QList<QCommandLineOption>{ pyramidOption, lOption }, doubleImg, a);
	// Если задан аргумент --morave или --harris
	processMoravecAndHarrisOption(sourceFileInfo.baseName(),  
		parser, QList<QCommandLineOption>{ moravicDetectorOption, harrisDetectorOption, anmsOption, sigmaOption}, doubleImg);

	auto end = timeCl::now();
	auto delta = std::chrono::duration_cast<timeMS>(end - start);
	if (parser.isSet(showInfoOption)) {
		std::cout << "Image Processing Complete(" << delta.count() << "ms)" << std::endl;
	}

	return 0;
}

void processLab1Option(const QCommandLineParser& parser, QList<QCommandLineOption>& options, DoubleMatrix& source) {
	if (!parser.isSet(options[0])) {
		return;
	}
	double sigma = parseDoubleOrDefault(parser.value(options[1]), 1.0);
	DoubleMatrix dx = source.dx();
	DoubleMatrix dy = source.dy();
	DoubleMatrix sobel = source.calcSobel();
	DoubleMatrix gauss = source.gaussian(sigma);

	LabImage::saveImage(dx, "out-dx.jpg");
	LabImage::saveImage(dy, "out-dy.jpg");
	LabImage::saveImage(sobel, "out-sobel.jpg");
	LabImage::saveImage(gauss, "out-gauss.jpg");
}

void processPyramidOption(const QCommandLineParser& parser, QList<QCommandLineOption>& options, DoubleMatrix& source, QCoreApplication& a) {
	QCommandLineOption& pyramidOption = options[0];
	QCommandLineOption& lOption = options[1];
	if (parser.isSet(pyramidOption)) {
		std::vector<double> pyramidVals = parseDoubleVector(parser.value(pyramidOption), ";");
		if (pyramidVals.size() == 4) {
			auto pyramid = Pyramid::createFrom(source, pyramidVals[0], pyramidVals[1], pyramidVals[2], pyramidVals[3]);
			pyramid.saveImage(a.applicationDirPath() + "\\pyramid");
			if (parser.isSet(lOption)) {
				std::vector<double> lVals = parseDoubleVector(parser.value(lOption), ";");
				if (lVals.size() == 3) {
						std::cout << "L(" << lVals[0] << "," << lVals[1] << "," << lVals[2] << "):" << std::endl;
						double l = pyramid.getPixel(lVals[0], lVals[1], lVals[2]);
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
}

void processMoravecAndHarrisOption(QString& srcName, const QCommandLineParser& parser, QList<QCommandLineOption>& options, DoubleMatrix& source) {
	QCommandLineOption& moravicDetectorOption = options[0];
	QCommandLineOption& harrisDetectorOption = options[1];
	QCommandLineOption& anmsOption = options[2];
	DoubleMatrix workImg = source.gaussian(parseDoubleOrDefault(parser.value(options[3]), 1.));
	double sigma = parseDoubleOrDefault(parser.value(options[3]), 1.0);
	int defaultAnmsPointCount = 500;
	bool withAnms = parser.isSet(anmsOption);
	int anmsPointCount = parseIntOrDefault(parser.value(anmsOption), defaultAnmsPointCount);
	char method = 0;

	if (parser.isSet(moravicDetectorOption)) {
		std::vector<double> params = parseDoubleVector(parser.value(moravicDetectorOption), ";");
		callCornerDetectorMethod(srcName, 'm', source, workImg, params, withAnms, anmsPointCount);
	}
	if (parser.isSet(harrisDetectorOption)) {
		std::vector<double> params = parseDoubleVector(parser.value(harrisDetectorOption), ";");
		callCornerDetectorMethod(srcName, 'h', source, workImg, params, withAnms, anmsPointCount);
	}
}

void callCornerDetectorMethod(QString& srcName, char method, DoubleMatrix& src, DoubleMatrix& img, std::vector<double> params, bool withANMS, int pointCount) {
	printValues({ "winSize", "pSize", "threshold" }, params);
	DoubleMatrix opImg;
	QString filename;
	QColor pointsColor;
	double winSize = params[0];
	double pSize = params[1];
	double threshold = params[2];
	if (method == 'm') {
		std::cout << "Moravec ";
		filename = "moravec";
		pointsColor = Qt::red;
		opImg = img.operatorMoravec(winSize);
	}
    else if (method == 'h') {
		std::cout << "Harris ";
		filename = "harris";
		pointsColor = Qt::green;
		opImg = img.operatorHarris(winSize);
	}
	std::vector<KeyPoint> points = opImg.getLocalMax(pSize, threshold);
	if (withANMS) {
		std::cout << "[ANMS]";
		filename += "-anms" + QString::number(pointCount);
		points = KeyPoint::anms(points, pointCount);
	}
	std::cout << " point count = " << points.size() << std::endl;
	QImage qimg = LabImage::getImageFromMatrix(DoubleMatrix(src).norm255());
	LabImage result(qimg);

	result.drawKeyPoints(points, pointsColor);
	result.save(srcName + "-" + filename + ".png");
}