//
// Created by Jur de Vries on 14/03/2018.
//

#ifndef PHOTONICDIRECTOR_DROPLET_H
#define PHOTONICDIRECTOR_DROPLET_H

#include "../Effects.h"
#include "cinder/Rand.h"

namespace photonic {

    class Droplet : public Effect {
    public:
        enum Inputs {
            kInput_InputChannel = 1,
            kInput_Theshold = 2,
            kInput_MinIntensity = 3,
            kInput_MaxIntensity = 4,
            kInput_DropInterval = 5,
            kInput_DropIntervalRandomness = 6,
            kInput_DecreaseSpeed = 7,
            kInput_BaseColor = 8,
            kInput_EffectColor = 9,
        };

        explicit Droplet(std::string name, std::string uuid = "");

        void execute(double dt) override;
        std::string getTypeName() override;
        std::string getTypeClassName() override;

    protected:
        LightRef getRandomLight();

        Timer mTimer;
        Rand mRand;
        double mInactiveTime;
        std::map<LightRef, float> mStartValues;
        std::map<LightRef, float> mCurrentValues;
        std::map<LightRef, float> mEndValues;
    };
}


#endif //PHOTONICDIRECTOR_DROPLET_H
