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
            kInput_Radius,
            kInput_DropOff,
            kInput_EffectColor,
            kInput_Intensity,
            kInput_EyeLocation,
            kInput_ViewDirection,
            kInput_HSLColorChannel,
            kInput_WidthChannel,
            kInput_DropOffChannel,
            kInput_ControllerVolumeChannel,
            kInput_InstrumentVolumeChannel,
            kInput_UseInstrumentInput,
        };
        FlashLight(std::string name, std::string uuid = "");

        void execute(double dt) override;
        std::string getTypeName() override;
        std::string getTypeClassName() override;
        void visualize() override;

    protected:
        vec3 mEyeLocation;
        vec3 mViewDirection;

        float calculateDistanceToLine(vec3 itemPosition, vec3 eyePosition, vec3 direction);
        float calculateDistanceToEye(vec3 itemPosition, vec3 eyePosition, vec3 direction);
        vec3 getNearestPointOnLine(vec3 itemPosition, vec3 eyePosition, vec3 direction);
    };

}


#endif //PHOTONICDIRECTOR_FLASHLIGHT_H
