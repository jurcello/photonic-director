//
// Created by Jur de Vries on 31/05/2018.
//

#ifndef PHOTONICDIRECTOR_PULSESTATIC_H
#define PHOTONICDIRECTOR_PULSESTATIC_H

#include "../Effects.h"

namespace photonic {

    class PulseStatic : public Effect {
    public:
        enum Inputs {
            kInput_Trigger = 1,
            kInput_Volume = 2,
            kInput_DecreaseTime = 3,
            kInput_BaseColor = 4,
            kInput_EffectColor = 5,
        };

        explicit PulseStatic(std::string name, std::string uuid = "");

        void execute(double dt) override;

        std::string getTypeClassName() override;

        std::string getTypeName() override;

    protected:
        void updateInnerState();

        float mCurrentValue;
        float mTargetValue;

        Timer mTimer;
    };
}


#endif //PHOTONICDIRECTOR_PULSESTATIC_H
