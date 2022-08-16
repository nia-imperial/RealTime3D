#include <QApplication>
#include "../../src/navigation/maininterface.h"
#include <QDebug>
#include <pybind11/embed.h> // everything needed for embedding

namespace py = pybind11;

int main(int argc, char *argv[]) {
    py::scoped_interpreter guard{};
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setOrganizationName(QStringLiteral(u"ImperialCollege"));
    QApplication::setOrganizationDomain(QStringLiteral(u"imperial.ac.uk"));
    QApplication::setApplicationName(QStringLiteral(u"Waypointer"));

    QApplication a(argc, argv);
    MainInterface w;
    w.show();
    return QApplication::exec();

}
