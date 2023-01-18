#include "Micro_Plastic_Analyzer.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	Micro_Plastic_Analyzer w;
	w.show();
	return a.exec();
}