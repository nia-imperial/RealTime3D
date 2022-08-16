//
// Created by arnoldn on 28/03/2022.
//

#ifndef REALTIME3D_GEOMETRICSETTINGS_H
#define REALTIME3D_GEOMETRICSETTINGS_H

#include "../SettingsForm.h"


enum InterpolationFlags {
    INTER_NEAREST = 0,
    INTER_LINEAR = 1,
    INTER_CUBIC = 2,
    INTER_AREA = 3,
    INTER_LANCZOS4 = 4,
    INTER_LINEAR_EXACT = 5,
    INTER_NEAREST_EXACT = 6,
    ITER_MAX = 7,
    WARP_FILL_OUTLIERS = 8,
    WARP_INVERSE_MAP = 9
};


QT_BEGIN_NAMESPACE
namespace Ui { class GeometricSettings; }
QT_END_NAMESPACE

class GeometricSettings : public SettingsForm {
Q_OBJECT

public:
    explicit GeometricSettings(QWidget *parent = nullptr);

    ~GeometricSettings() override;

    void writeSettings() override;

    void readSettings() override;

    static SettingDescriptor desc;

    QMap<InterpolationFlags, QString> methodDesc;

    InterpolationFlags methodLookup(int idx);

    void resetToDefault() override;

    static int getInterMethod();

private:
    Ui::GeometricSettings *ui;

    void addMethod(const QString &text, InterpolationFlags flag, const QString &descript);

    void setMethod(InterpolationFlags method);

    int getIndex(InterpolationFlags method);
};


#endif //REALTIME3D_GEOMETRICSETTINGS_H
