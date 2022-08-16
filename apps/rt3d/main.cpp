#include <QApplication>
#include "../../src/main_window/main_window.h"
#include <QDebug>
#include <pybind11/embed.h> // everything needed for embedding

namespace py = pybind11;

int main(int argc, char *argv[]) {
    py::scoped_interpreter guard{};
    py::gil_scoped_release release;

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setOrganizationName(QStringLiteral(u"ImperialCollege"));
    QApplication::setOrganizationDomain(QStringLiteral(u"imperial.ac.uk"));
    QApplication::setApplicationName(QStringLiteral(u"RT3D"));

    if (GeneralSettings::getOpenGLValue()) {
        QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    }

    QApplication a(argc, argv);
    auto message = Messages();
    qInstallMessageHandler(Messages::msgHandler);

    if (GeneralSettings::getOpenGLValue()) qInfo() << "Using OpenGL";

    auto w = MainWindow(message.getConsoleWidget());
    w.show();
    return QApplication::exec();

}
