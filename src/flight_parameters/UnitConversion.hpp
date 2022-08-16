//
// Created by Nic on 24/01/2022.
//

#ifndef REALTIME3D_UNITCONVERSION_HPP
#define REALTIME3D_UNITCONVERSION_HPP

namespace units::metres::to {
    auto feet(double m) {
        return m * 3.28084;
    };

    auto km(double ms) {
        return ms * 3.6;
    };
}

namespace units::metres::from {
    auto feet(double ft) {
        return ft / 3.28084;
    };

    auto km(double km) {
        return km / 1000;
    };
}

namespace units::metres::per::second::to {
    auto kmh(double ms) {
        return ms * 3.6;
    };
    auto fps(double ms) {
        return ms * 3.28084;
    }
    auto mph(double ms){
        return ms * 2.23694;
    }
}

namespace units::metres::per::second::from {
    auto kmh(double kmh) {
        return kmh / 3.6;
    };
    auto fps(double fps) {
        return fps / 3.28084;
    }
    auto mph(double mph){
        return mph / 2.23694;
    }
}


#endif //REALTIME3D_UNITCONVERSION_HPP
