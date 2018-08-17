//
// Created by Jur de Vries on 20/06/2018.
//

#ifndef PHOTONICDIRECTOR_DRIFTER_H
#define PHOTONICDIRECTOR_DRIFTER_H

#include "../Effects.h"
#include "cinder/Rand.h"

namespace photonic {

    class Drifter : public Effect{
    public:
        enum Inputs {
            kInput_EffectColor = 1,
            kInput_InputChannel = 2,
            kInput_BaseSpeed = 3,
            kInput_MaxSpeed = 4,
            kInput_BaseFluctuation = 5,
            kInput_Radius = 6,
            kInput_DropOff = 7,
            kInput_FixedVolume = 8,
            kInput_SpeedThreshold = 9,
        };

        Drifter(std::string name, std::string uuid = "");

        void execute(double dt) override;
        void init() override ;
        std::string getTypeName() override;
        std::string getTypeClassName() override;

    protected:
        void setNewTargetPosition();
        void calculateCurrentPosition(double dt);
        void updateTargetPosition();
        vec3 getRandomPosition();
        vec3 getDirectionVector();
        float getSpeed();

        Rand mRand;
        vec3 mCurrentPosition;
        vec3 mTargetPosition;
    };
}

#endif //PHOTONICDIRECTOR_DRIFTER_H
