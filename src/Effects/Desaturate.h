//
// Created by Jur de Vries on 14/03/2018.
//

#ifndef PHOTONICDIRECTOR_DESATURATE_H
#define PHOTONICDIRECTOR_DESATURATE_H

#include "../Effects.h"

namespace photonic {

    class Desaturate : public Effect {
    public:
        enum Inputs {
            kInput_InputChannel,
        };

        explicit Desaturate(std::string name, std::string uuid = "");

        void execute(double dt) override;
        std::string getTypeName() override;
        std::string getTypeClassName() override;
        Effect::Stage getStage() override;

    };
}


#endif //PHOTONICDIRECTOR_DESATURATE_H
