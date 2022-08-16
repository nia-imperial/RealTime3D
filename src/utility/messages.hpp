//
// Created by arnoldn on 20/05/2021.
//

#ifndef REALTIME3D_MESSAGES_HPP
#define REALTIME3D_MESSAGES_HPP

#include <QWidget>
#include <QPlainTextEdit>
#include <QFileSystemWatcher>

class Messages : public QObject {
Q_OBJECT
    QPlainTextEdit *consoleWidget;
    QFile logFile;
public:
    explicit Messages(QObject *parent = nullptr);

    static void warning_msg(QWidget *parent, const QString &infoStr);

    static void info_msg(QWidget *parent, const QString &infoStr);

    static void msgHandler(QtMsgType type, const QMessageLogContext &context, const QString &message);

    static QString currentLogfile();

    QPlainTextEdit *getConsoleWidget();

};

#endif //REALTIME3D_MESSAGES_HPP
