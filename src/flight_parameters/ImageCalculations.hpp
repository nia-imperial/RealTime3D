//
// Created by Nic on 24/01/2022.
//

#ifndef REALTIME3D_IMAGECALCULATIONS_HPP
#define REALTIME3D_IMAGECALCULATIONS_HPP

#include <cmath>
#include <numbers>

namespace image {

    namespace diagonal {
        double equivalent() {
            return sqrt(24 * 24 + 36 * 36);
        }

        double pixels(double imageWidth, double imageHeight) {
            return sqrt(pow(imageWidth, 2) + pow(imageHeight, 2));
        }

        double degrees(double focalLength) {
            return 180 * 2 * atan(diagonal::equivalent() / (2 * focalLength)) / std::numbers::pi;
        }

        double meters(double heightAboveGround, double focalLength) {
            return heightAboveGround * tan(std::numbers::pi * degrees(focalLength) / (2 * 180));
        }

    }

    namespace motionBlur {
        double cm(double flightSpeed_ms, double shutterSpeed) {
            return 100 * flightSpeed_ms / shutterSpeed;
        }

        double pixels(double flightSpeed_ms, double shutterSpeed) {
            return (100 * flightSpeed_ms / shutterSpeed) / cm(flightSpeed_ms, shutterSpeed);
        }

    }

    namespace interval {
        double meters(double flightSpeed_ms, double interval_s) {
            return flightSpeed_ms * interval_s;
        }
    }

    double angularResolution(double imageWidth, double imageHeight, double focalLength) {
        return diagonal::degrees(focalLength) / diagonal::pixels(imageWidth, imageHeight);
    }

    namespace groundPixelSize {
        double cm(double imageWidth, double imageHeight, double heightAboveGround, double focalLength) {
            return diagonal::meters(heightAboveGround, focalLength) / diagonal::pixels(imageWidth, imageHeight);
        }
    }

    namespace groundSize {
        double metres(double imageAxisSize, double heightAboveGround, double focalLength) {
            return imageAxisSize * diagonal::meters(heightAboveGround, focalLength);
        }
    }

    namespace overlap {
        double meters(double imageWidth, double imageHeight, double heightAboveGround, double focalLength) {
            auto widthMetres = groundSize::metres(imageWidth, heightAboveGround, focalLength);
            auto heightMetres = groundSize::metres(imageWidth, heightAboveGround, focalLength);
            return widthMetres - heightMetres;
        }

        double percent(double imageWidth, double imageHeight, double heightAboveGround, double focalLength) {
            auto image_meters = meters(imageWidth, imageHeight, heightAboveGround, focalLength);
            return 100 * image_meters / groundSize::metres(imageWidth, heightAboveGround, focalLength);
        }
    }

    namespace rotationBlur {
        double rb1(double imageWidth, double imageHeight, double rotationLensAxis, double shutterSpeed) {
            auto diag_px = diagonal::pixels(imageWidth, imageHeight);
            return (rotationLensAxis * shutterSpeed) * diag_px * std::numbers::pi / 360;
        }

        double rb2(double imageWidth, double imageHeight,
                   double focalLength, double rotationOrthogonal, double shutterSpeed) {
            auto diag_degrees = diagonal::degrees(focalLength);
            auto diag_px = diagonal::pixels(imageWidth, imageHeight);
            return (rotationOrthogonal / shutterSpeed) / (diag_degrees / diag_px);
        }
    }

}

#endif //REALTIME3D_IMAGECALCULATIONS_HPP
