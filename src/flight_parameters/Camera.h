//
// Created by Nic on 18/01/2022.
//

#ifndef REALTIME3DPROJECT_CAMERA_H
#define REALTIME3DPROJECT_CAMERA_H

#include <tuple>

struct Resolution {
    double x;
    double y;
};

struct Sensor {
    double x;
    double y;
};

struct Lens {
    double focal_length;
};

class Camera {
public:
    Resolution res;
    Sensor sensor;
    Lens lens;

    /**
    * The angle of view of a camera can be calculated from the camera’s
    * effective focal length.
    * @param f The focal length in mm
    * @param d The dimension of the sensor for a given axis in mm
    * @return angle of view
     */
    static double findAngleOfView(double f, double d);

    /**
     * Calculate Meters per Pixel for a given axis
     * @param imageRes The camera/image resolution for the axis in pixels.
     * @param cameraHeight The camera's height above the ground in metres.
     * @param focalLength The effective focal length of the lens in millimeters.
     * @param sensorDimSize The size of the sensor in millimeters for the axis.
     * @return metres per pixel (m / px)
     */
    static double calc_mpp(int imageRes, double cameraHeight, double focalLength, double sensorDimSize);

    /**
     * The Pixel per metres for the camera at a given height. Both x and y axis are returned.
     * @param cameraHeight The camera's height above the ground in metres.
     * @return Meters per pixel (m / px) of both the x and y axis.
     */
    std::tuple<double, double> ppm(double cameraHeight);
};


#endif //REALTIME3DPROJECT_CAMERA_H
