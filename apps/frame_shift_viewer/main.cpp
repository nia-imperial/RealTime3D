#include <QApplication>
#include <QDebug>
#include "../../src/frame_shift_viewer/FrameShiftModule.h"
#include "../../src/settings/general_settings/generalsettings.h"


int main(int argc, char *argv[]) {
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setOrganizationName(QStringLiteral(u"ImperialCollege"));
    QApplication::setOrganizationDomain(QStringLiteral(u"imperial.ac.uk"));
    QApplication::setApplicationName(QStringLiteral(u"RT3D"));

    if (GeneralSettings::getOpenGLValue()) {
        QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
        qInfo() << "Using OpenGL";
    }

    QApplication a(argc, argv);
    FrameShiftModule w;
    w.show();
    return QApplication::exec();

}

