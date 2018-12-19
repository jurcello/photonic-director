//
// Created by Jur de Vries on 13/12/2018.
//

#ifndef PHOTONICDIRECTOR_FIGHTINGINPUTS_H
#define PHOTONICDIRECTOR_FIGHTINGINPUTS_H

#include "../Effects.h"
#include "CinderImGui.h"

namespace photonic {
    struct IntensityTracker {
        float intensity = 0.f;
        float stepSize = 0.f;

        void update(float newIntensity);

    };

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
            kInput_MaxLightsIntensiy = 10,
            kInput_MaxLightsFadeOutTime = 11,
            kInput_NegativeRadiusCausingZero = 12,
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

        // Intensity trackers.
        IntensityTracker mVictimIntensityTracker;
        IntensityTracker mAttackerIntensityTracker;

        float mVictimOwnLampIntensityFactor;
        float mVictimLampIntensity;

        void UpdateRadius(double dt);
        void updateVictimLamp();
        void updateIntensityTrackers(double dt);

        void reset();
    };
}

#endif //PHOTONICDIRECTOR_FIGHTINGINPUTS_H
