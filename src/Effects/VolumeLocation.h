//
// Created by Jur de Vries on 22/02/2018.
//

#ifndef PHOTONICDIRECTOR_VOLUMELOCATION_H
#define PHOTONICDIRECTOR_VOLUMELOCATION_H

#include "../Effects.h"

namespace photonic {
    class VolumeLocation : public Effect {
    public:
        enum Inputs {
            kInput_LocationChannel = 1,
            kInput_DropOff = 2,
            kInput_Radius = 3,
            kInput_Intensity = 4,
            kInput_BaseColor = 5,
            kInput_EffectColor = 6,
            kInput_TriggerChannel = 7,
        };

        VolumeLocation(std::string name, std::string uuid = "");

        void execute(double dt) override;
        std::string getTypeName() override;
        std::string getTypeClassName() override;
        void visualize() override;

    protected:
        vec3 mLocation;
        vec3 mStaticLocation;

    };
}


#endif //PHOTONICDIRECTOR_VOLUMELOCATION_H
