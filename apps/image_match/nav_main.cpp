#include <QString>
#include "../../src/navigation/imageprocessing.h"

int main(int argc, char *argv[]) {

    auto nadirPath = QString("C:/Users/Nic/PycharmProjects/navigation_py/data/nadir1.jpg");
    auto obliquePath = QString("C:/Users/Nic/PycharmProjects/navigation_py/data/oblique2.jpg");

    auto imageMatcher = ImageMatcher(
            R"(C:\Tiger\Data\Output\PerspectiveMatches)",
            true);
    auto outImage = imageMatcher.match(nadirPath, obliquePath);
    imageMatcher.writeImage(outImage, "nadir_to_oblique.png");

}