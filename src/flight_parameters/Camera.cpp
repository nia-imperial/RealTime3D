//
// Created by Nic on 18/01/2022.
//

#include <cmath>
#include "Camera.h"


double Camera::findAngleOfView(double f, double d) {
    return 2 * atan(d / (2 * f));
}

double Camera::calc_mpp(int imageRes, double cameraHeight, double focalLength, double sensorDimSize) {
    focalLength /= 10;
    auto AoV = findAngleOfView(focalLength, sensorDimSize);

    // the tangent of half the angle of view is equal to
    // half the ratio of the ground distance and the height
    auto groundDistance = 2 * cameraHeight * tan(AoV / 2);

    return groundDistance / imageRes;
}

std::tuple<double, double> Camera::ppm(double cameraHeight) {
    auto mu_x = calc_mpp(res.x, cameraHeight, lens.focal_length, sensor.x);

    auto mu_y = calc_mpp(res.y, cameraHeight, lens.focal_length, sensor.y);

    auto mu = std::make_tuple(mu_x, mu_y);
    return mu;

}
