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
            kInput_Volume = 2,
            kInput_Theshold = 3,
            kInput_MinIntensity = 4,
            kInput_MaxIntensity = 5,
            kInput_DropInterval = 6,
            kInput_DropIntervalRandomness = 7,
            kInput_DecreaseSpeed = 8,
            kInput_BaseColor = 9,
            kInput_EffectColor = 10,
            kInput_VolumeChannel = 11,
            kInput_DropIntervalChannel = 12,
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
