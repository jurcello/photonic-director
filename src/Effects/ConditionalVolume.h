//
// Created by Jur de Vries on 19/04/2018.
//

#ifndef PHOTONICDIRECTOR_CONDITIONALVOLUME_H
#define PHOTONICDIRECTOR_CONDITIONALVOLUME_H

#include "../Effects.h"

namespace photonic {
    class ConditionalVolume : public Effect {
    public:
        enum Inputs {
            kInput_VolumeChannel = 1,
            kInput_ConditionChannel = 2,
            kInput_Min = 3,
            kInput_Max = 4,
            kInput_OutsideFade = 5,
            kInput_BaseColor = 6,
            kInput_EffectColor = 7,
        };
        ConditionalVolume(std::string name, std::string uuid = "");

        void execute(double dt) override;
        std::string getTypeName() override;
        std::string getTypeClassName() override;


    protected:
        Timer mTimer;
        bool mIsOn;
        bool mWasOn;
        bool mIsFading;
        float intensityDuringFading;
        float mFadeFactor;

        void updateStatus();
    };
}


#endif //PHOTONICDIRECTOR_CONDITIONALVOLUME_H
