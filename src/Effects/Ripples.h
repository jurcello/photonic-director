//
// Created by Jur de Vries on 05/03/2018.
//

#ifndef PHOTONICDIRECTOR_RIPPLES_H
#define PHOTONICDIRECTOR_RIPPLES_H

#include "../Effects.h"
#include "cinder/Perlin.h"

namespace photonic {
    class Ripples : public Effect {
    public:
        enum Inputs {
            kInput_BaseColor = 1,
            kInput_EffectColor = 2,
            kInput_EffectVolume = 3,
            kInput_NoiseAmount = 4,
            kInput_NoiseSpeed = 5,
            kInput_ExternalVolume = 6,
            kInput_ExternalNoiseAmount = 7,
            kInput_ExternalNoiseSpeed = 8,
        };
        Ripples(std::string name, std::string uuid = "");

        void execute(double dt) override;
        void init() override ;
        std::string getTypeName() override;
        std::string getTypeClassName() override;

    protected:
        Perlin mPerlin;
        Timer mTimer;
    };
}


#endif //PHOTONICDIRECTOR_RIPPLES_H
