//
// Created by Nic on 21/01/2022.
//

#ifndef REALTIME3D_FLIGHTPARAMETERS_H
#define REALTIME3D_FLIGHTPARAMETERS_H

#include <QWidget>
#include <QDialog>
#include <QDebug>
#include <ostream>

QT_BEGIN_NAMESPACE
namespace Ui { class FlightParameters; }
QT_END_NAMESPACE

class FlightParameters : public QDialog {
Q_OBJECT

public:
    explicit FlightParameters(QWidget *parent = nullptr);

    ~FlightParameters() override;

    enum class ShotType {
        INTERVAL, BASELINE
    };
    enum class SpeedUnit {
        /// metres per second.
        ms,
        /// kilometres per hour.
        kmh,
        /// feet per second.
        fps,
        /// miles per hour.
        mph
    };

    enum class DistUnit {
        /// metres.
        m,
        /// feet.
        ft
    };

    enum class BlurUnit {
        /// centimetres
        cm,
        /// pixels
        px
    };

    enum class OverlapUnit {
        percent,
        metres
    };

public Q_SLOTS:

    // field changes
    void handle_imageWidthSpin(int val);

    void handle_imageHeightSpin(int val);

    void handle_shutterSpeedSpin(int val);

    void handle_apertureSpin(double val);

    void handle_isoSpin(int val);

    void handle_shootingTypeSpin(double val);

    void handle_focalLengthSpin(double val);

    void handle_sensorWidthSpin(double val);

    void handle_sensorHeightSpin(double val);

    void handle_speedSpin(double val);

    void handle_heightSpin(double val);

    void handle_groundPixelSizeSpin(double val);

    void handle_imageIntvSizeSpin(double val);

    void handle_imageOverlapSpin(double val);

    void handle_imageGroundSizeXSpin(double val);

    void handle_imageGroundSizeYSpin(double val);

    void handle_motionBlurSpin(double val);

    // field setters
    void setImageIntervalSize();

    void setImageOverlap();

    void setGroundPixelSize();

    void setImageGroundSize();

    void setMotionBlur();

    // metric adapters
    void changeShootingType(int index);

    void changeSpeedUnit(int index);

    void changeHeightUnit(int index);

    void changeOverlapUnit(int index);

    void changeMotionBlurUnit(int index);

protected:
    void closeEvent(QCloseEvent *event) override;
private:
    /// Interval between images given in seconds.
    double imageInterval;

    /// Distance between camera positions in metres.
    double cameraBaseline;

    /// Speed of the UAV in metres per second.
    double uavSpeed{};

    /// The UAV's above ground height in metres.
    double uavHeight{};

    void writeSettings();

    void readSettings();

    Ui::FlightParameters *ui;

    void calculate();
    double diagonalViewAngle{};
    double angularResolution{};

    double rotationLensAxis{};
    double rotationOrthogonal{};
    double rotationBlur1{};
    double rotationBlur2{};

};


#endif //REALTIME3D_FLIGHTPARAMETERS_H
