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
            kInput_ColorChannel = 7,
            kInput_WidthChannel = 8,
            kInput_DropOffChannel = 9,
            kInput_ControllerVolumeChannel = 10,
            kInput_InstrumentVolumeChannel = 11,
            kInput_UseInstrumentInput = 12,
        };
        FlashLight(std::string name, std::string uuid = "");

        void execute(double dt) override;
        std::string getTypeName() override;
        std::string getTypeClassName() override;
        void visualize() override;

    protected:
        vec3 mEyeLocation;
        vec3 mViewDirection;
        float mEffectIntensity;

        void updateStaticValuesWithChannels();
        float calculateDistanceToLine(vec3 itemPosition, vec3 eyePosition, vec3 direction);
        float calculateDistanceToEye(vec3 itemPosition, vec3 eyePosition, vec3 direction);
        vec3 getNearestPointOnLine(vec3 itemPosition, vec3 eyePosition, vec3 direction);

        void updateColorFromExternalSignal();
        void updateEffectIntensity();
    };

}


#endif //PHOTONICDIRECTOR_FLASHLIGHT_H
