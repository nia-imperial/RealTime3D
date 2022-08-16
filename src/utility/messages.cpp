#include "messages.hpp"
#include <QMessageBox>
#include <QFile>
#include <QMutex>
#include <iostream>
#include <QApplication>
#include <QDir>


Messages::Messages(QObject *parent) :
        QObject(parent),
        logFile(QFile(currentLogfile())),
        consoleWidget(new QPlainTextEdit) {

    qSetMessagePattern(QString("[%{type}]: %{message}"));

    if (QFileInfo::exists(logFile.fileName()))
        logFile.resize(0);

    consoleWidget->setObjectName(QLatin1String("ConsoleWidget"));
    consoleWidget->setReadOnly(true);
    logFile.open(QFile::ReadOnly);

}

void Messages::warning_msg(QWidget *parent, const QString &infoStr) {
    QMessageBox::warning(parent, "Warning Message", infoStr);
}

void Messages::info_msg(QWidget *parent, const QString &infoStr) {
    QMessageBox::information(parent, "Notification", infoStr);
}

QString Messages::currentLogfile() {
    auto logDir = QDir(QApplication::applicationDirPath()).filePath("logs");
    if (not QFileInfo::exists(logDir))
        QDir(logDir).mkpath(".");
    return QDir(logDir).filePath("current_log.txt");
}

void Messages::msgHandler(QtMsgType type, const QMessageLogContext &context, const QString &message) {
    static QMutex mutex;
    QMutexLocker lock(&mutex);

    static QFile logFile(currentLogfile());
    static bool logFileIsOpen = logFile.open(QIODevice::Append | QIODevice::Text);

    // print to standard err
    std::cerr << qPrintable(qFormatLogMessage(type, context, message)) << std::endl;

    // hack: print to console widget
    auto topWidgets = QApplication::allWidgets();
    for (auto w: topWidgets) {
        if (w->objectName() == QStringLiteral("MainWindow")){
            auto consoleWidget = w->findChild<QPlainTextEdit *>(QLatin1String("ConsoleWidget"));
            if (consoleWidget)
                consoleWidget->insertPlainText(qPrintable(qFormatLogMessage(type, context, message) + "\n"));
            break;
        }
    }

    // print to log file
    if (logFileIsOpen) {
        logFile.write(qFormatLogMessage(type, context, message).toUtf8() + '\n');
        logFile.flush();
    }
}

QPlainTextEdit *Messages::getConsoleWidget() {
    return consoleWidget;
}


