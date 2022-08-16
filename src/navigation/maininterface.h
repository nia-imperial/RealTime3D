//
// Created by Nic on 22/02/2022.
//

#ifndef WAYPTR_MAININTERFACE_H
#define WAYPTR_MAININTERFACE_H

#include <QMainWindow>
#include <QPointer>
#include "layerpanel.h"
#include "coordinatepanel.h"
#include "Workspace/workspace.h"
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QVBoxLayout>
#include "flighttools.h"
#include "waypointer_io/images.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainInterface; }
QT_END_NAMESPACE

class FlightImageDialog : public QDialog {
Q_OBJECT
public:
    QLineEdit *flightFileLine;
    QSpinBox *xIn;
    QSpinBox *yIn;

    explicit FlightImageDialog(QWidget *parent) :
            QDialog(parent),
            flightFileLine(new QLineEdit(QLatin1String("..."))),
            xIn(new QSpinBox()),
            yIn(new QSpinBox()) {
        setModal(Qt::ApplicationModal);
        auto dialogLayout = new QVBoxLayout();

        auto grid1 = new QGridLayout();
        auto flightFileLabel = new QLabel(QLatin1String("Flight File:"));

        auto flightFileSelect = new QPushButton(QLatin1String("Select"));
        connect(flightFileSelect, &QPushButton::released,
                this, [this]() {
                    auto file = QFileDialog::getOpenFileName(parentWidget(), QLatin1String("Flight Image"),
                                                             QLatin1String("C:/Tiger/Data/Input"),
                                                             supportedImageFormats());
                    if (file.isEmpty() or file.isNull())
                        flightFileLine->setText(QLatin1String("..."));
                    else
                        flightFileLine->setText(file);
                });
        grid1->addWidget(flightFileLabel, 0, 0);
        grid1->addWidget(flightFileLine, 0, 1);
        grid1->addWidget(flightFileSelect, 0, 2);
        dialogLayout->addLayout(grid1);

        auto xLayout = new QHBoxLayout();
        xLayout->addWidget(new QLabel(QLatin1String("X:")));

        xIn->setMaximum(9999);
        xIn->setMinimum(-9999);
        xLayout->addWidget(xIn);

        auto yLayout = new QHBoxLayout();
        yLayout->addWidget(new QLabel(QLatin1String("Y:")));
        yIn->setMaximum(9999);
        yIn->setMinimum(-9999);
        yLayout->addWidget(yIn);

        auto xyLayout = new QHBoxLayout();
        xyLayout->addLayout(xLayout);
        xyLayout->addLayout(yLayout);
        xyLayout->setAlignment(Qt::AlignLeft);

        auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                              | QDialogButtonBox::Cancel,
                                              Qt::Horizontal);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        dialogLayout->addWidget(buttonBox);

        setLayout(dialogLayout);
    }
};

class MainInterface : public QMainWindow {
Q_OBJECT

public:
    explicit MainInterface(QWidget *parent = nullptr);

    ~MainInterface() override = default;

    LayerPanel *layerPanel;
    CoordinatePanel *coordinatePanel;
    Workspace *workspace;
    FlightTools *flightTools;

    QString waypointFile;
    QString baseItemFile;

public Q_SLOTS:

    void openBasemapPhoto(const QString &filePath = "");

    void openWaypoints(const QString &filePath = "") const;

    void addFlightImage(QString flightImagePath = QString(), int x = 0, int y = 0);

    void saveWaypoints();


    void setZ(double z) const;

protected:
    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

private:
    Ui::MainInterface *ui;

    void saveTargetPoints();
};


#endif //WAYPTR_MAININTERFACE_H
