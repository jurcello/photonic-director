//
// Created by Jur de Vries on 04/05/2018.
//

#ifndef PHOTONICDIRECTOR_CLOSEINLOCATION_H
#define PHOTONICDIRECTOR_CLOSEINLOCATION_H

#include "../Effects.h"

namespace photonic {
    class CloseInLocation : public Effect {
    public:
        enum Inputs {
            kInput_Location = 1,
            kInput_LocationChannel = 2,
            kInput_Intensity = 3,
            kInput_IntensityChannel = 4,
            kInput_DropOff = 5,
            kInput_Radius = 6,
            kInput_RadiusChannel = 7,
            kInput_EffectColor = 8,
        };

        CloseInLocation(std::string name, std::string uuid = "");

        void execute(double dt) override;
        std::string getTypeName() override;
        std::string getTypeClassName() override;
        void visualize() override;

    protected:
        vec3 mLocation;

    };
}


#endif //PHOTONICDIRECTOR_CLOSEINLOCATION_H
