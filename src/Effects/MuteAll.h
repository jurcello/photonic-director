//
// Created by Jur de Vries on 13/03/2018.
//

#ifndef PHOTONICDIRECTOR_MUTEALL_H
#define PHOTONICDIRECTOR_MUTEALL_H

#include "../Effects.h"

namespace photonic {

    class MuteAll : public Effect {
    public:
        enum Inputs {
            kInput_Input = 1,
            kInput_ExportChannel = 2,
            kInput_OffTreshold = 3,
        };
        MuteAll(std::string name, std::string uuid = "");

        virtual void execute(double dt) override;
        virtual std::string getTypeName() override;
        virtual std::string getTypeClassName() override;
        virtual Effect::Stage getStage() override;

    };
}

#endif //PHOTONICDIRECTOR_MUTEALL_H
