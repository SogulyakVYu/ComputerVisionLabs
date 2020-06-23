#pragma once
#include <QtCore/QCoreApplication>
#include <QtGui>
#include <vector>
#include "DoubleMatrix.h"
class ImgProgram
{
private:
	QCommandLineParser& parser;
	QCommandLineOption borderOption;
	QCommandLineOption sigmaOption;
	QCommandLineOption pyramidOption;
	QCommandLineOption lOption;
	QCommandLineOption showInfoOption;
	QCommandLineOption lab1Option;
	QCommandLineOption moravecDetectorOption;
	QCommandLineOption harrisDetectorOption;
	QCommandLineOption anmsOption;
	QCommandLineOption descriptorOption;
	QCommandLineOption thresholdOption;
	QCommandLineOption savePyramidsOption;

	QStringList posArgs;
	QString applicationDirPath;
	QStringList sourceFilesNames;
	QList<QFileInfo> sourceFilesInfo;

	bool isSet(const QCommandLineOption& option) { return parser.isSet(option); }
	QString value(const QCommandLineOption& option) { return parser.value(option); }

	void processLab1Option(DoubleMatrix& source);
	void processPyramidOption(DoubleMatrix& source);
	void processMoravecAndHarrisOption(DoubleMatrix& source);
	void callCornerDetectorMethod(char method, DoubleMatrix& source, DoubleMatrix& img, std::vector<double> params, int pointCount);
	void processDescriptorOption(DoubleMatrix& source1, DoubleMatrix source2);

	void processLab6Option(DoubleMatrix& source1, DoubleMatrix& source2);

	double getThreshold(double dflt = 0.6);

	template<typename T>
	static void printValues(const std::vector<std::string>& text, const std::vector<T>& values);

	static int parseIntOrDefault(const QString& line, int dflt);
	static double parseDoubleOrDefault(const QString& line, double dflt);
	static std::vector<double> parseDoubleVector(const QString& line, const QString& delim);

public:
	ImgProgram(QCommandLineParser& parser);
	void processParser(const QCoreApplication& app);
	void processOptions();
};

template<typename T>
inline void ImgProgram::printValues(const std::vector<std::string>& text, const std::vector<T>& values)
{
	int len = values.size();
	std::cout << text[0] << ": " << values[0];
	for (int i = 1; i < len; i++) {
		std::cout << ", " << text[i] << ": " << values[i];
	}
	std::cout << std::endl;
}
