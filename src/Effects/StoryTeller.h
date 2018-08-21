//
// Created by Jur de Vries on 21/08/2018.
//

#ifndef PHOTONICDIRECTOR_STORYTELLER_H
#define PHOTONICDIRECTOR_STORYTELLER_H

#include "../Effects.h"
#include "../Utils.h"
#include "CinderImGui.h"
#include "cinder/gl/gl.h"

namespace photonic {

    class StoryTeller: public Effect {

    public:
        enum Inputs {
            kInput_RotationChannelRight = 1,
            kInput_RotationChannelLeft = 2,
            kInput_Intensity = 3,
            kInput_IntensityChannel = 4,
            kInput_Width = 5,
            kInput_RotationComponent = 6,
        };

        StoryTeller(std::string name, std::string uuid = "");

        void execute(double dt) override;
        void init() override;
        void drawEditGui() override;
        void visualize() override;
        std::string getTypeName() override;
        std::string getTypeClassName() override;

    protected:
        float mCurrentRotationLeft;
        float mCurrentRotationRight;
        bool mHasRotationLeft, mHasRotationRight;
        float mCurrentIntensity;
        vec3 mCenter;
        vec3 mDirectionReference;
        bool mCenterCalculated;

        void updateState();
        void updateCurrentRotations();
        void calculateCenter();
        float calculateAngleDistance(float angle1, float angle2);
        float angleBetween(vec3 vector1, vec3 vector2);
    };

}


#endif //PHOTONICDIRECTOR_STORYTELLER_H
