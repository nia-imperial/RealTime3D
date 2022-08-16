//
// Created by Nic on 18/01/2022.
//

#ifndef REALTIME3DPROJECT_UAV_H
#define REALTIME3DPROJECT_UAV_H

#include <tuple>
#include "Camera.h"

class Uav {
public:
    double flightHeight;
    double flightSpeed;
    Camera *camera;

    void attachCamera(Camera &cam);
    void detachCamera();

    std::tuple<double, double> current_ppm();
};


#endif //REALTIME3DPROJECT_UAV_H
