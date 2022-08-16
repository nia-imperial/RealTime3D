//
// Created by arnoldn on 20/05/2021.
//


#include <thread>
#include "scriptlauncher.h"


ScriptLauncher::ScriptLauncher(QWidget *parent) :
        QWidget(parent),
        activeProcs(), procError(false),
        procsLaunched(0), procsReturned(0),
        procTimes() {
    setObjectName("ScriptLauncher");

    connect(this, &ScriptLauncher::procFinished,
            this, [this]() {
                if (procQueue.isEmpty()) return;
                auto nextProc = procQueue.takeFirst();
                nextProc->start();
                nextProc->waitForStarted();
            });

}

QProcess *ScriptLauncher::createNewProc() {
    auto process = new QProcess(this);
    process->setProcessChannelMode(QProcess::ForwardedChannels);

    connect(process, &QProcess::started,
            this, [=, this]() {
                auto pid = QString::number(process->processId());
                using namespace std::chrono;
                procTimes[pid.toStdString()] = duration_cast<seconds>(
                        steady_clock::now().time_since_epoch()).count();

                process->setObjectName(pid);
                appendProc(pid);
                Q_EMIT procStarted(pid);
                qInfo() << "Started process: PID " << process->objectName();
            });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=, this](int exitCode, QProcess::ExitStatus exitStatus) {
                auto pid = process->objectName();
                popProc(pid);

                using namespace std::chrono;
                auto end = duration_cast<seconds>(steady_clock::now().time_since_epoch()).count();

                auto st = procTimes.at(pid.toStdString());
                auto diff = end - st;
                qInfo() << "Process:" << process->objectName()
                        << " finished in" << diff << "(sec.)."
                        << "Exit code: " << exitCode;

                Q_EMIT resultReady(exitCode, QString(exitStatus));
                Q_EMIT procFinished(process->objectName());
                Q_EMIT procFinishedPID(process->objectName().toInt());
                if (numLaunched() == numReturned())
                        Q_EMIT allFinished();
            });

    connect(process, &QProcess::errorOccurred,
            this, [=, this](QProcess::ProcessError error) {
                procError = true;
                qWarning() << "Error " << error << " " << process->program() << " " << process->arguments();
            });

    return process;
}


void ScriptLauncher::killProcs() {
    auto processes = findChildren<QProcess *>();
    for (auto &proc: processes) {
        proc->terminate();
        if (!proc->waitForFinished(100))
            proc->kill();
        proc->waitForFinished();
        proc->deleteLater();
    }
}

void ScriptLauncher::launchProc(const QString &workingDir, const QString &program, const QStringList &scriptArgs) {
    auto process = createNewProc();
    process->setWorkingDirectory(workingDir);
    process->setProgram(program);
    process->setArguments(scriptArgs);
    if (numLaunched() == 0) {
        process->start();
        process->waitForStarted();
    } else {
        procQueue.append(process);
    }
}

void ScriptLauncher::appendProc(const QString &pid) {
    try {
        activeProcs.append(pid);
        procsLaunched += 1;
    } catch (std::runtime_error &e) {
        Messages::warning_msg(this, e.what());
    }
}


void ScriptLauncher::popProc(const QString &pid) {
    try {
        activeProcs.removeOne(pid);
        procsReturned += 1;
    } catch (std::runtime_error &e) {
        Messages::warning_msg(this, e.what());
    }
}

void ScriptLauncher::reset() {
    procsLaunched = 0;
    procsReturned = 0;
    procError = false;
}

[[maybe_unused]] int ScriptLauncher::numLaunched() const {
    return procsLaunched;
}

int ScriptLauncher::numReturned() const {
    return procsReturned;
}

void ScriptLauncher::closeEvent(QCloseEvent *event) {
    killProcs();
    QWidget::closeEvent(event);
}
