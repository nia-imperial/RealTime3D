//
// Created by Nic on 02/03/2022.
//

#ifndef REALTIME3D_PYSCRIPTCALLER_H
#define REALTIME3D_PYSCRIPTCALLER_H

#include <string>

namespace pycall {
    bool dat2tiff(const std::string &filename);

    bool dat2tiff_dir(const std::string &dirname);

    std::string perspectiveMatching(const std::string &outputDir,
                                    const std::string &nadirImagePath,
                                    const std::string &obliqueImagePath,
                                    bool saveImages = true);


    std::tuple<int, int> matchAerialToMap(const std::string &aerialImage,
                                          const std::string &baseMap,
                                          int x, int y);

    void video2frames(const std::string &videoFilePath,
                      const std::string &framesDir,
                      double frameRate);

    void correctDistortion(const std::string &imagePath,
                           const std::string &cameraMaker,
                           const std::string &cameraModel,
                           const std::string &lensMaker,
                           const std::string &lensModel,
                           double focalLength,
                           double aperture,
                           double distanceToSubject,
                           int interMethod);

    std::string makeFramesDir(const std::string &videoFilePath);

    void video2frames(const std::string &videoFilePath, const std::string &framesDir, double frameRate, int maximum);
}


#endif //REALTIME3D_PYSCRIPTCALLER_H
