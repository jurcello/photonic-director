//
// Created by Jur de Vries on 13/09/2018.
//

#ifndef PHOTONICDIRECTOR_MOVINGHEAD_H
#define PHOTONICDIRECTOR_MOVINGHEAD_H

#include "../Effects.h"
#include "../Utils.h"
#include "../LightComponent.h"

namespace photonic {

    class MovingHead: public Effect {
    public:
        enum Inputs {
            kInput_Intensity = 1,
            kInput_Pan = 2,
            kInput_Tilt = 3,
            kInput_Focus = 4,
            kInput_IntensityChannel = 5,
            kInput_PanChannel = 6,
            kInput_TiltChannel = 7,
            kInput_IrisChannel = 8,
            kInput_FocusChannel = 9,
        };

        MovingHead(std::string name, std::string uuid = "");

        bool supportsLight(LightRef light) override;
        void execute(double dt) override;
        std::string getTypeName() override;
        std::string getTypeClassName() override;

    protected:

    };
}

#endif //PHOTONICDIRECTOR_MOVINGHEAD_H
