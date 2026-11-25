#include "WeldingDiagnosis.h"
#include <QtWidgets/QApplication>
#include <QtCharts>
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	WeldingDiagnosis w;
	w.show();
	return a.exec();
}
