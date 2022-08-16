//
// Created by Nic on 24/05/2021.
//

#ifndef REALTIME3D_GROUPFILEOBJECTDIALOG_HPP
#define REALTIME3D_GROUPFILEOBJECTDIALOG_HPP

#include <QGuiApplication>
#include <QScreen>
#include <QFileDialog>
#include <filesystem>
#include "../navigation/waypointer_io/images.hpp"

namespace fs = std::filesystem;

class GroupFileObjectDialog : public QFileDialog {
    Q_OBJECT
    const QStringList builtin_filters = {"All (*)",
                                         "Directories (*/)",
                                         supportedImageFormats(),
                                         supportedVideoFormats()};
public:
    GroupFileObjectDialog(QWidget *parent, const QString &caption, const QString &directory) :
            QFileDialog(parent, caption, directory) {
        setOption(QFileDialog::DontUseNativeDialog);
        setFileMode(QFileDialog::ExistingFile);
        resize(QGuiApplication::primaryScreen()->availableSize() * 2 / 5);
        QObject::connect(this, &QFileDialog::currentChanged,
                         this, &GroupFileObjectDialog::selected);
    };

private Q_SLOTS:

    void selected(const QString &name) {
        fs::path path_name = name.toStdString();
        if (is_directory(path_name)) {
            setFileMode(QFileDialog::Directory);
        } else
            setFileMode(QFileDialog::ExistingFile);
        setNameFilters(builtin_filters);
    }
};


#endif //REALTIME3D_GROUPFILEOBJECTDIALOG_HPP
