//
// Created by Jur de Vries on 19/02/2018.
//

#ifndef PHOTONICDIRECTOR_FLASHLIGHT_H
#define PHOTONICDIRECTOR_FLASHLIGHT_H

#include "../Effects.h"


namespace photonic {
    class FlashLight : public Effect {
    public:
        enum Inputs {
            kInput_Radius = 1,
            kInput_DropOff = 2,
            kInput_EffectColor = 3,
            kInput_Intensity = 4,
            kInput_EyeLocation = 5,
            kInput_ViewDirection = 6,
        };
        FlashLight(std::string name, std::string uuid = "");

        void execute(double dt) override;
        std::string getTypeName() override;
        std::string getTypeClassName() override;

    protected:
        vec3 mEyeLocation;
        vec3 mViewDirection;

        float calculateDistanceToLine(vec3 itemPosition, vec3 eyePosition, vec3 direction);
        float calculateDistanceToEye(vec3 itemPosition, vec3 eyePosition, vec3 direction);
    };

}


#endif //PHOTONICDIRECTOR_FLASHLIGHT_H
