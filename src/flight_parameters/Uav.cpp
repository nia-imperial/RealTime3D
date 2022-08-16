//
// Created by Nic on 18/01/2022.
//

#include "Uav.h"

void Uav::attachCamera(Camera &cam) {
    camera = &cam;
}

void Uav::detachCamera() {
    camera = nullptr;
}

std::tuple<double, double> Uav::current_ppm() {
    return camera->ppm(flightHeight);
}
