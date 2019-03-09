//
// Created by Jur de Vries on 14/02/2018.
//

#ifndef PHOTONICDIRECTOR_LOCATIONFOLLOW_H
#define PHOTONICDIRECTOR_LOCATIONFOLLOW_H

#include "../Effects.h"

namespace photonic {
    class LocationFollow : public Effect {
    public:
        enum Inputs {
            kInput_EffectColor = 1,
            kInput_Radius = 2,
            kInput_Intensity = 3,
            kInput_DropOff = 4,
            kInput_FadeOutTime = 5,
        };
        LocationFollow(std::string name, std::string uuid = "");

        virtual void execute(double dt) override;
        void init() override ;
        virtual std::string getTypeName() override;
        virtual std::string getTypeClassName() override;

    protected:

    };
}

#endif //PHOTONICDIRECTOR_LOCATIONFOLLOW_H
