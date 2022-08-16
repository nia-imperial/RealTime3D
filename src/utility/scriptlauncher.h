//
// Created by arnoldn on 20/05/2021.
//

#ifndef REALTIME3D_SCRIPTLAUNCHER_HPP
#define REALTIME3D_SCRIPTLAUNCHER_HPP

#include <QWidget>
#include <QProcess>
#include "messages.hpp"
#include <QTextCodec>
#include <QDebug>
#include <iostream>
#include <map>
#include <chrono>

//extra: handle error launched procs

class ScriptLauncher : public QWidget {
Q_OBJECT

    QProcess *createNewProc();

    QList<QString> activeProcs;
    QList<QProcess *> procQueue;
    std::map<std::string, long long int> procTimes;
    int procsLaunched;
    int procsReturned;

public:
    explicit ScriptLauncher(QWidget *parent);

protected:
    void closeEvent(QCloseEvent *event) override;

public:
    bool procError;

    [[maybe_unused]] [[nodiscard]] int numLaunched() const;

    [[nodiscard]] int numReturned() const;

Q_SIGNALS:

    void resultReady(int exitCode, const QString &msg);

    void messageChanged(const QString &msg);

    void procStarted(const QString &pid);

    void procFinishedPID(int pid);

    void procFinished(const QString &pid);

    void allFinished();

public Q_SLOTS:

    void launchProc(const QString &workingDir, const QString &program, const QStringList &scriptArgs);

    void killProcs();

    void reset();

private Q_SLOTS:

    void appendProc(const QString &pid);

    void popProc(const QString &pid);


};


#endif //REALTIME3D_SCRIPTLAUNCHER_HPP
