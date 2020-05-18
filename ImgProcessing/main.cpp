#include <QtCore/QCoreApplication>

#include "ImgProgram.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QCommandLineParser parser;
	ImgProgram prog(parser);

	prog.processParser(a);
	prog.processOptions();

	return 0;
}