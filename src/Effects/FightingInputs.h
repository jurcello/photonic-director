//
// Created by Jur de Vries on 13/12/2018.
//

#ifndef PHOTONICDIRECTOR_FIGHTINGINPUTS_H
#define PHOTONICDIRECTOR_FIGHTINGINPUTS_H

#include "../Effects.h"
#include "CinderImGui.h"

namespace photonic {
    class FightingInputs : public Effect {
    public:
        enum Inputs {
            kInput_VictimChannel = 1,
            kInput_AttackerChannel = 2,
            kInput_VictimColor = 3,
            kInput_AttackerColor = 4,
            kInput_VictimLamp = 5,
            kInput_VictimIncreaseSpeed = 6,
            kInput_AttackerDecreaseSpeed = 7,
            kInput_AttackerStartRadius = 8,
            kInput_DropOff = 9,
        };

        FightingInputs(std::string name, std::string uuid = "");

        void init() override;
        void execute(double dt) override;
        std::string getTypeName() override;
        std::string getTypeClassName() override;
        void drawEditGui() override;

    protected:
        float mVictimRadius;
        float mAttackerRadius;

        float mVictimOwnLampIntensityFactor;
        float mVictimLampIntensity;

        void UpdateRadius(double dt);
        void updateVictimLamp();

    };
}

#endif //PHOTONICDIRECTOR_FIGHTINGINPUTS_H
