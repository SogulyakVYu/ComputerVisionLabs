#include <iostream>
#include <chrono>
#include "ImgProgram.h"
#include "LabImage.h"
#include "Pyramid.h"
#include "KeyPointHelper.h"
#include "DescriptorExtractor.h"

using chronoClock = std::chrono::high_resolution_clock;
using chronoMs = std::chrono::milliseconds;


void ImgProgram::processLab1Option(DoubleMatrix& source)
{
	if (!isSet(lab1Option)) return;
	double sigma = parseDoubleOrDefault(parser.value(sigmaOption), 1.0);
	DoubleMatrix dx = source.dx();
	DoubleMatrix dy = source.dy();
	DoubleMatrix sobel = source.calcSobel();
	DoubleMatrix gauss = source.gaussian(sigma);

	LabImage::saveImage(dx, "out-dx.jpg");
	LabImage::saveImage(dy, "out-dy.jpg");
	LabImage::saveImage(sobel, "out-sobel.jpg");
	LabImage::saveImage(gauss, "out-gauss.jpg");
}

void ImgProgram::processPyramidOption(DoubleMatrix& source)
{
	if (parser.isSet(pyramidOption)) {
		std::vector<double> pyramidVals = parseDoubleVector(parser.value(pyramidOption), ";");
		if (pyramidVals.size() == 4) {
			auto pyramid = Pyramid::createFrom(source, pyramidVals[0], pyramidVals[1], pyramidVals[2], pyramidVals[3]);
			pyramid.saveImage(applicationDirPath + "\\pyramid");
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

void ImgProgram::processMoravecAndHarrisOption(DoubleMatrix& source)
{
	if (isSet(descriptorOption)) {
		return;
	}
	DoubleMatrix workImg(source);
	int defaultAnmsPointCount = 500;
	bool withAnms = parser.isSet(anmsOption);
	int anmsPointCount = parseIntOrDefault(parser.value(anmsOption), defaultAnmsPointCount);
	char method = 0;
	if (isSet(sigmaOption)) {
		double sigma = parseDoubleOrDefault(parser.value(sigmaOption), 1.);
		workImg = workImg.gaussian(sigma);
	}

	if (isSet(moravecDetectorOption)) {
		std::vector<double> params = parseDoubleVector(parser.value(moravecDetectorOption), ";");
		callCornerDetectorMethod('m', source, workImg, params, anmsPointCount);
	}
	if (parser.isSet(harrisDetectorOption)) {
		std::vector<double> params = parseDoubleVector(parser.value(harrisDetectorOption), ";");
		callCornerDetectorMethod('h', source, workImg, params, anmsPointCount);
	}
}

void ImgProgram::callCornerDetectorMethod(char method, DoubleMatrix& source, DoubleMatrix& img, std::vector<double> params, int pointCount)
{
	DoubleMatrix opImg;
	QString filename;
	QColor pointsColor;
	double winSize = params[0];
	double pSize = params[1];
	double threshold = params[2];
	bool withAnms = isSet(anmsOption);
	if (method == 'm') {
		std::cout << "Moravec";
		filename = "moravec";
		pointsColor = Qt::red;
		opImg = img.operatorMoravec(winSize);
	}
	else if (method == 'h') {
		std::cout << "Harris";
		filename = "harris";
		pointsColor = Qt::green;
		opImg = img.operatorHarris(winSize);
	}
	std::vector<KeyPoint> points = KeyPointHelper::getLocalMax(opImg, pSize, threshold);
	if (withAnms) {
		std::cout << "[ANMS]";
		filename += "-anms" + QString::number(pointCount);
		points = KeyPointHelper::anms(points, pointCount);
	}
	std::cout << " point count = " << points.size() << std::endl;
	printValues({ "winSize", "pSize", "threshold" }, params);
	QImage qimg = LabImage::getImageFromMatrix(DoubleMatrix(source).norm255());
	LabImage result(qimg);

	result.drawKeyPoints(points, pointsColor);
	result.save(sourceFilesNames[0] + "-" + filename + ".png");
}

void ImgProgram::processDescriptorOption(DoubleMatrix& source1, DoubleMatrix source2)
{
	if (!isSet(descriptorOption)) {
		return;
	}
	DoubleMatrix workImg(source1);
	DoubleMatrix workImg2(source2);

	if (isSet(sigmaOption)) {
		double sigma = parseDoubleOrDefault(value(sigmaOption), 0);
		workImg = workImg.gaussian(sigma);
		workImg2 = workImg2.gaussian(sigma);
	}

	std::vector<double> descriptorParams = parseDoubleVector(value(descriptorOption), ";");
	int gridSize = descriptorParams[0];
	int cellCount = descriptorParams[1];
	int binCount = descriptorParams[2];

	std::vector<double> operatorParams;
	char cornerOp = 0;

	if (isSet(moravecDetectorOption)) {
		cornerOp = 'm';
		operatorParams = parseDoubleVector(value(moravecDetectorOption), ";");
	}
	else if (isSet(harrisDetectorOption)) {
		cornerOp = 'h';
		operatorParams = parseDoubleVector(value(harrisDetectorOption), ";");
	}
	else {
		std::cout << "corner operator not set" << std::endl;
		return;
	}

	auto kp = KeyPointHelper::getLocalMax(workImg.operatorHarris(operatorParams[0]), operatorParams[1], operatorParams[2]);
	auto kp2 = KeyPointHelper::getLocalMax(workImg2.operatorHarris(operatorParams[0]), operatorParams[1], operatorParams[2]);
	if (isSet(anmsOption)) {
		int pointCount = parseIntOrDefault(value(anmsOption), 0);
		kp = KeyPointHelper::anms(kp, pointCount);
		kp2 = KeyPointHelper::anms(kp2, pointCount);
	}

	DescriptorExtractor extractor(gridSize, cellCount, binCount);
	std::vector<Descriptor> ds = extractor.compute(workImg, kp);
	std::vector<Descriptor> ds2 = extractor.compute(workImg2, kp2);

	double threshold = 0.66;
	if (isSet(thresholdOption)) {
		threshold = parseDoubleOrDefault(value(thresholdOption), threshold);
	}

	auto matches = DescriptorExtractor::findMatches(ds, ds2, threshold);
	QImage mainCopy = LabImage::getImageFromMatrix(source1.norm255());
	QImage secondCopy = LabImage::getImageFromMatrix(source2.norm255());

	std::vector<QColor> colors = LabImage::getRandomColors(kp.size());
	LabImage::drawKeyPoints(mainCopy, kp, Qt::red, 3);
	LabImage::drawKeyPoints(secondCopy, kp2, Qt::red, 3);
	QImage resultImg = LabImage::joinImages(mainCopy, secondCopy);
	LabImage::drawMatches(resultImg, mainCopy.width(), 0, matches, kp, kp2, colors);

	resultImg.save("match-" + sourceFilesInfo[0].baseName() + "-" + sourceFilesInfo[1].baseName() + ".png");
}

int ImgProgram::parseIntOrDefault(const QString& line, int dflt)
{
	bool isInt = false;
	int val = line.toInt(&isInt);
	int result = isInt ? val : dflt;
	return result;
}

double ImgProgram::parseDoubleOrDefault(const QString& line, double dflt)
{
	bool isDouble = false;
	double val = line.toDouble(&isDouble);
	double result = isDouble ? val : dflt;
	return result;
}

std::vector<double> ImgProgram::parseDoubleVector(const QString& line, const QString& delim)
{
	std::vector<double> result(0);
	QStringList words = line.split(delim);
	for (QString& word : words) {
		bool isDouble = false;
		double val = word.toDouble(&isDouble);
		if (isDouble) result.push_back(val);
	}

	return result;
}

ImgProgram::ImgProgram(QCommandLineParser& parser) :
	parser(parser),
	borderOption("border", "Image border type (0 - black, 1 - border pixel, 2 - reflect (default), 3 - wrap )", "borderType", "2"),
	sigmaOption("sigma", "Sigma value for gauss filter (default = 1.0)", "sigmaVal", "1.0"),
	pyramidOption("pyramid", "Create pyramid from 'sigmaA;sigma0;octaveCount;levelCount'", "pyramidVal"),
	lOption("L", "L=x;y;sigma", "lVal"),
	showInfoOption("info", "Shows file size and image processing time"),
	lab1Option("Lab1", "Create dx,dy, gauss, sobel image"),
	moravecDetectorOption("moravec", "Moravec corner detector 'winSize;localMaxWinSize;threshold", "moravecVal"),
	harrisDetectorOption("harris", "Harris corner detector 'winSize;localMaxWinSize;threshold", "harrisVal"),
	anmsOption("anms", "ANMS filter ", "anmsVal"),
	descriptorOption("descriptor", "Simple Descriptor 'gridSize;cellCount;binCount", "descriptorVal"),
	thresholdOption("t", "Threshold for some methods", "thresholdVal")
{
	parser.addHelpOption();
	parser.addPositionalArgument("source", "Source Image(images)");

	parser.addOption(borderOption);
	parser.addOption(sigmaOption);
	parser.addOption(pyramidOption);
	parser.addOption(lOption);
	parser.addOption(showInfoOption);
	parser.addOption(lab1Option);
	parser.addOption(moravecDetectorOption);
	parser.addOption(harrisDetectorOption);
	parser.addOption(anmsOption);
	parser.addOption(descriptorOption);
	parser.addOption(thresholdOption);
}

void ImgProgram::processParser(const QCoreApplication& app)
{
	parser.process(app);
	posArgs = parser.positionalArguments();

	QStringList sourceFiles = posArgs[0].split(";");
	for (QString& fileName : sourceFiles) {
		sourceFilesNames.append(fileName);
		sourceFilesInfo.append(QFileInfo(fileName));
	}
	applicationDirPath = app.applicationDirPath();
}

void ImgProgram::processOptions()
{
	for (QFileInfo& fileInfo : sourceFilesInfo) {
		if (!fileInfo.exists()) {
			std::cout << "File " << fileInfo.fileName().toStdString() << " - file doesn't exist" << std::endl;
			return;
		}
	}

	QImage qFirstImage(sourceFilesInfo[0].absoluteFilePath());
	LabImage labFirstImage(qFirstImage);
	IntMatrix intFirstImg = IntMatrix::fromImage(labFirstImage.getGrayScale());
	DoubleMatrix doubleFirstImg = intFirstImg.toDoubleMatrix();
	if (isSet(showInfoOption)) {
		std::cout << "File: " << sourceFilesInfo[0].fileName().toStdString() << " ";
		labFirstImage.printInfo();
	}

	QImage qSecondImage;
	DoubleMatrix doubleSecondImg;
	if (sourceFilesInfo.size() > 1) {
		QImage img(sourceFilesInfo[1].absoluteFilePath());
		qSecondImage = img.copy();
		LabImage labSecondImage(img);
		doubleSecondImg = labSecondImage.getDoubleMatrix();
		if (isSet(showInfoOption)) {
			std::cout << "Second File: " << sourceFilesInfo[1].fileName().toStdString() << " ";
			labSecondImage.printInfo();
		}

		doubleSecondImg.norm1();
	}

	doubleFirstImg.norm1();

	auto startTime = chronoClock::now();

	processLab1Option(doubleFirstImg);
	processPyramidOption(doubleFirstImg);
	processMoravecAndHarrisOption(doubleFirstImg);
	processDescriptorOption(doubleFirstImg, doubleSecondImg);

	auto endTime = chronoClock::now();
	auto deltaTime = std::chrono::duration_cast<chronoMs>(endTime - startTime);

	if (isSet(showInfoOption)) {
		std::cout << " --- Image Processing Complete(" << deltaTime.count() << "ms) ---" << std::endl;
	}
}
